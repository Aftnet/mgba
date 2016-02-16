/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "core/config.h"
#include "core/thread.h"
#include "gba/core.h"
#include "gba/gba.h"
#include "gba/renderers/video-software.h"
#include "gba/serialize.h"

#include "platform/commandline.h"
#include "util/string.h"
#include "util/vfs.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/time.h>

#define PERF_OPTIONS "F:L:NPS:"
#define PERF_USAGE \
	"\nBenchmark options:\n" \
	"  -F FRAMES        Run for the specified number of FRAMES before exiting\n" \
	"  -N               Disable video rendering entirely\n" \
	"  -P               CSV output, useful for parsing\n" \
	"  -S SEC           Run for SEC in-game seconds before exiting\n" \
	"  -L FILE          Load a savestate when starting the test"

struct PerfOpts {
	bool noVideo;
	bool csv;
	unsigned duration;
	unsigned frames;
	char* savestate;
};

static void _GBAPerfRunloop(struct mCoreThread* context, int* frames, bool quiet);
static void _GBAPerfShutdown(int signal);
static bool _parsePerfOpts(struct mSubParser* parser, int option, const char* arg);
static void _loadSavestate(struct mCoreThread* context);

static struct mCoreThread* _thread;
static bool _dispatchExiting = false;
static struct VFile* _savestate = 0;

int main(int argc, char** argv) {
	signal(SIGINT, _GBAPerfShutdown);

	struct PerfOpts perfOpts = { false, false, 0, 0, 0 };
	struct mSubParser subparser = {
		.usage = PERF_USAGE,
		.parse = _parsePerfOpts,
		.extraOptions = PERF_OPTIONS,
		.opts = &perfOpts
	};

	struct mArguments args;
	bool parsed = parseArguments(&args, argc, argv, &subparser);
	if (!parsed || args.showHelp) {
		usage(argv[0], PERF_USAGE);
		freeArguments(&args);
		return !parsed;
	}
	if (args.showVersion) {
		version(argv[0]);
		freeArguments(&args);
		return 0;
	}

	void* outputBuffer = malloc(256 * 256 * 4);

	struct mCore* core = GBACoreCreate();
	struct mCoreThread context = {
		.core = core
	};
	_thread = &context;

	if (!perfOpts.noVideo) {
		core->setVideoBuffer(core, outputBuffer, 256);
	}
	if (perfOpts.savestate) {
		_savestate = VFileOpen(perfOpts.savestate, O_RDONLY);
		free(perfOpts.savestate);
	}
	if (_savestate) {
		context.startCallback = _loadSavestate;
	}

	// TODO: Put back debugger
	char gameCode[5] = { 0 };

	core->init(core);
	mCoreLoadFile(core, args.fname);
	mCoreConfigInit(&core->config, "perf");
	mCoreConfigLoad(&core->config);

	mCoreConfigSetDefaultIntValue(&core->config, "idleOptimization", IDLE_LOOP_REMOVE);
	struct mCoreOptions opts;
	mCoreConfigMap(&core->config, &opts);
	opts.audioSync = false;
	opts.videoSync = false;
	applyArguments(&args, NULL, &core->config);
	mCoreConfigLoadDefaults(&core->config, &opts);
	mCoreLoadConfig(core);

	int didStart = mCoreThreadStart(&context);

	if (!didStart) {
		goto cleanup;
	}
	mCoreThreadInterrupt(&context);
	if (mCoreThreadHasCrashed(&context)) {
		mCoreThreadJoin(&context);
		goto cleanup;
	}

	GBAGetGameCode(core->board, gameCode);
	mCoreThreadContinue(&context);

	int frames = perfOpts.frames;
	if (!frames) {
		frames = perfOpts.duration * 60;
	}
	struct timeval tv;
	gettimeofday(&tv, 0);
	uint64_t start = 1000000LL * tv.tv_sec + tv.tv_usec;
	_GBAPerfRunloop(&context, &frames, perfOpts.csv);
	gettimeofday(&tv, 0);
	uint64_t end = 1000000LL * tv.tv_sec + tv.tv_usec;
	uint64_t duration = end - start;

	mCoreThreadJoin(&context);

	float scaledFrames = frames * 1000000.f;
	if (perfOpts.csv) {
		puts("game_code,frames,duration,renderer");
		const char* rendererName;
		if (perfOpts.noVideo) {
			rendererName = "none";
		} else {
			rendererName = "software";
		}
		printf("%s,%i,%" PRIu64 ",%s\n", gameCode, frames, duration, rendererName);
	} else {
		printf("%u frames in %" PRIu64 " microseconds: %g fps (%gx)\n", frames, duration, scaledFrames / duration, scaledFrames / (duration * 60.f));
	}

cleanup:
	if (_savestate) {
		_savestate->close(_savestate);
	}
	mCoreConfigFreeOpts(&opts);
	freeArguments(&args);
	mCoreConfigDeinit(&core->config);
	free(outputBuffer);

	return !didStart || mCoreThreadHasCrashed(&context);
}

static void _GBAPerfRunloop(struct mCoreThread* context, int* frames, bool quiet) {
	struct timeval lastEcho;
	gettimeofday(&lastEcho, 0);
	int duration = *frames;
	*frames = 0;
	int lastFrames = 0;
	while (context->state < THREAD_EXITING) {
		if (mCoreSyncWaitFrameStart(&context->sync)) {
			++*frames;
			++lastFrames;
			if (!quiet) {
				struct timeval currentTime;
				long timeDiff;
				gettimeofday(&currentTime, 0);
				timeDiff = currentTime.tv_sec - lastEcho.tv_sec;
				timeDiff *= 1000;
				timeDiff += (currentTime.tv_usec - lastEcho.tv_usec) / 1000;
				if (timeDiff >= 1000) {
					printf("\033[2K\rCurrent FPS: %g (%gx)", lastFrames / (timeDiff / 1000.0f), lastFrames / (float) (60 * (timeDiff / 1000.0f)));
					fflush(stdout);
					lastEcho = currentTime;
					lastFrames = 0;
				}
			}
		}
		mCoreSyncWaitFrameEnd(&context->sync);
		if (duration > 0 && *frames == duration) {
			_GBAPerfShutdown(0);
		}
		if (_dispatchExiting) {
			mCoreThreadEnd(context);
		}
	}
	if (!quiet) {
		printf("\033[2K\r");
	}
}

static void _GBAPerfShutdown(int signal) {
	UNUSED(signal);
	// This will come in ON the GBA thread, so we have to handle it carefully
	_dispatchExiting = true;
	ConditionWake(&_thread->sync.videoFrameAvailableCond);
}

static bool _parsePerfOpts(struct mSubParser* parser, int option, const char* arg) {
	struct PerfOpts* opts = parser->opts;
	errno = 0;
	switch (option) {
	case 'F':
		opts->frames = strtoul(arg, 0, 10);
		return !errno;
	case 'N':
		opts->noVideo = true;
		return true;
	case 'P':
		opts->csv = true;
		return true;
	case 'S':
		opts->duration = strtoul(arg, 0, 10);
		return !errno;
	case 'L':
		opts->savestate = strdup(arg);
		return true;
	default:
		return false;
	}
}

static void _loadSavestate(struct mCoreThread* context) {
	context->core->loadState(context->core, _savestate, 0);
	_savestate->close(_savestate);
	_savestate = 0;
}
