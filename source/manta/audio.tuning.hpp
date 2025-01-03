#pragma once

namespace SysAudioTuning
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#define LPF_ORDER 2 // TODO

	constexpr int numCombs = 8;
	constexpr int numAllpasses = 4;
	constexpr float	muted = 0.0f;
	constexpr float	fixedGain = 0.015f;
	constexpr float scaleWet = 3.0f;
	constexpr float scaleDry = 2.0f;
	constexpr float scaleDamp = 0.4f;
	constexpr float scaleRoom = 0.28f;
	constexpr float offsetRoom = 0.7f;
	constexpr float initialRoom = 0.9f;
	constexpr float initialDamp = 0.5f;
	constexpr float initialWet = 1.0f / scaleWet;
	constexpr float initialDry = 1.0f;
	constexpr float initialWidth = 1.0f;
	constexpr float initialMode = 0.0f;
	constexpr float freezeMode = 0.5f;
	constexpr int stereoSpread = 23;

	// These values assume 44.1KHz sample rate
	// they will probably be OK for 48KHz sample rate
	// but would need scaling for 96KHz (or other) sample rates.
	// The values were obtained by listening tests.
	constexpr int combTuningL1 = 1116;
	constexpr int combTuningR1 = 1116 + stereoSpread;
	constexpr int combTuningL2 = 1188;
	constexpr int combTuningR2 = 1188 + stereoSpread;
	constexpr int combTuningL3 = 1277;
	constexpr int combTuningR3 = 1277 + stereoSpread;
	constexpr int combTuningL4 = 1356;
	constexpr int combTuningR4 = 1356 + stereoSpread;
	constexpr int combTuningL5 = 1422;
	constexpr int combTuningR5 = 1422 + stereoSpread;
	constexpr int combTuningL6 = 1491;
	constexpr int combTuningR6 = 1491 + stereoSpread;
	constexpr int combTuningL7 = 1557;
	constexpr int combTuningR7 = 1557 + stereoSpread;
	constexpr int combTuningL8 = 1617;
	constexpr int combTuningR8 = 1617 + stereoSpread;
	constexpr int allpassTuningL1 = 556;
	constexpr int allpassTuningR1 = 556 + stereoSpread;
	constexpr int allpassTuningL2 = 441;
	constexpr int allpassTuningR2 = 441 + stereoSpread;
	constexpr int allpassTuningL3 = 341;
	constexpr int allpassTuningR3 = 341 + stereoSpread;
	constexpr int allpassTuningL4 = 225;
	constexpr int allpassTuningR4 = 225 + stereoSpread;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}