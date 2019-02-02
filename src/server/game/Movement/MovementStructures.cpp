#include "MovementStructures.h"
#include "Player.h"

MovementStatusElements const PlayerMove[] =
{
    MSEHasMovementFlags,       // 24
    MSEMovementFlags,          // 24
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte6,           // 22
    MSEOneBit,                 // 168
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte1,           // 17
    MSEZeroBit,                // 149
    MSEZeroBit,                // 172
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasOrientation,         // 48
    MSEHasMovementFlags2,      // 28
    MSEHasPitch,               // 112
    MSEMovementFlags2,         // 28
    MSEHasGuidByte7,           // 23
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte2,           // 18
    MSEHasFallData,            // 140
    MSEHasTransportData,       // 104
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasFallDirection,       // 136
    MSEZeroBit,                // 148
    MSEHasGuidByte3,           // 19
    MSEHasTimestamp,           // 32
    MSETransportTime2,         // 88
    MSETransportPositionX,     // 64
    MSETransportGuidByte6,     // 62
    MSETransportOrientation,   // 76
    MSETransportPositionZ,     // 72
    MSETransportTime3,         // 96
    MSETransportGuidByte0,     // 56
    MSETransportTime,          // 84
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte2,     // 58
    MSETransportSeat,          // 80
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte5,     // 61
    MSETransportPositionY,     // 68
    MSETransportGuidByte4,     // 60
    MSEGuidByte3,              // 19
    MSEPitch,                  // 112
    MSEGuidByte6,              // 22
    MSEPositionX,              // 36
    MSEGuidByte4,              // 20
    MSEMovementForcesId,            // 152
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSEPositionY,              // 40
    MSESplineElevation,        // 144
    MSETimestamp,              // 32
    MSEGuidByte1,              // 17
    MSEGuidByte2,              // 18
    MSEGuidByte5,              // 21
    MSEGuidByte0,              // 16
    MSEPositionZ,              // 44
    MSEGuidByte7,              // 23
    MSEOrientation,            // 48
    MSEEnd

};

MovementStatusElements const SetCanFly[] =
{
    MSEPositionY,
    MSEPositionX,
    MSEPositionZ,
    MSEHasGuidByte7,
    MSEZeroBit,
    MSEOneBit,
    MSEHasGuidByte5,
    MSEZeroBit,
    MSEHasMovementFlags,
    MSERemoveMovementForcesCount,
    MSEHasGuidByte2,
    MSEHasTransportData,
    MSEHasTimestamp,
    MSEHasGuidByte6,
    MSEHasFallData,
    MSEHasPitch,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasSplineElevation,
    MSEHasGuidByte0,
    MSEHasOrientation,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSEHasMovementFlags2,
    MSEMovementFlags,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte1,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEGuidByte1,
    MSEGuidByte0,
    MSEMovementForcesId,
    MSEGuidByte7,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEGuidByte2,
    MSEPitch,
    
    MSETransportGuidByte3,
    MSETransportTime2,
    MSETransportPositionY,
    MSETransportGuidByte7,
    MSETransportGuidByte2,
    MSETransportPositionX,
    MSETransportSeat,
    MSETransportTime3,
    MSETransportGuidByte6,
    MSETransportPositionZ,
    MSETransportTime,
    MSETransportGuidByte5,
    MSETransportGuidByte4,
    MSETransportGuidByte0,
    MSETransportGuidByte1,
    MSETransportOrientation,
    MSEFallVerticalSpeed,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallTime,
    MSESplineElevation,
    MSETimestamp,
    MSEOrientation,
    MSEEnd

};

MovementStatusElements const MovementFallLand[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEHasTimestamp,           // 32
    MSEHasGuidByte6,           // 22
    MSEHasFallData,            // 140
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasUnkTime,             // 168
    MSEHasMovementFlags,       // 24
    MSEZeroBit,                // 149
    MSEHasPitch,               // 112
    MSEZeroBit,                // 172
    MSEZeroBit,                // 148
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte3,           // 19
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte1,           // 17
    MSEHasTransportData,       // 104
    MSEHasGuidByte7,           // 23
    MSEHasOrientation,         // 48
    MSERemoveMovementForcesCount,           // 152
    MSEMovementFlags,          // 24
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime3,      // 100
    MSEMovementFlags2,         // 28
    MSEHasFallDirection,       // 136
    MSEGuidByte4,              // 20
    MSEGuidByte0,              // 16
    MSEGuidByte7,              // 23
    MSEGuidByte6,              // 22
    MSEGuidByte2,              // 18
    MSEGuidByte5,              // 21
    MSEGuidByte1,              // 17
    MSEMovementForcesId,            // 156
    MSEGuidByte3,              // 19
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSETransportTime2,         // 88
    MSETransportGuidByte5,     // 61
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte7,     // 63
    MSETransportTime,          // 84
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte1,     // 57
    MSETransportOrientation,   // 76
    MSETransportGuidByte6,     // 62
    MSETransportPositionY,     // 68
    MSETransportSeat,          // 80
    MSETransportGuidByte0,     // 56
    MSETransportPositionX,     // 64
    MSETransportGuidByte4,     // 60
    MSETransportPositionZ,     // 72
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementHeartBeat[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte7,           // 23
    MSEHasUnkTime,             // 168
    MSEHasGuidByte1,           // 17
    MSEHasFallData,            // 140
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte3,           // 19
    MSEZeroBit,                // 172
    MSEHasMovementFlags,       // 24
    MSEZeroBit,                // 148
    MSEZeroBit,                // 149
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte6,           // 22
    MSEHasTimestamp,           // 32
    MSEHasTransportData,       // 104
    MSEHasGuidByte2,           // 18
    MSERemoveMovementForcesCount,           // 152
    MSEHasPitch,               // 112
    MSEHasSplineElevation,     // 144
    MSEHasOrientation,         // 48
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEHasFallDirection,       // 136
    MSEGuidByte1,              // 17
    MSEGuidByte3,              // 19
    MSEGuidByte7,              // 23
    MSEGuidByte4,              // 20
    MSEGuidByte5,              // 21
    MSEGuidByte0,              // 16
    MSEGuidByte6,              // 22
    MSEMovementForcesId,            // 156
    MSEGuidByte2,              // 18
    MSEUnkTime,                // 168
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSETransportTime,          // 84
    MSETransportGuidByte1,     // 57
    MSETransportPositionZ,     // 72
    MSETransportGuidByte2,     // 58
    MSETransportTime2,         // 88
    MSETransportOrientation,   // 76
    MSETransportPositionY,     // 68
    MSETransportPositionX,     // 64
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte4,     // 60
    MSETransportSeat,          // 80
    MSETransportGuidByte5,     // 61
    MSEPitch,                  // 112
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementJump[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEHasPitch,               // 112
    MSEHasOrientation,         // 48
    MSEHasGuidByte7,           // 23
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 172
    MSEHasMovementFlags2,      // 28
    MSEHasUnkTime,             // 168
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasTransportData,       // 104
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 149
    MSEHasTimestamp,           // 32
    MSEZeroBit,                // 148
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte6,           // 22
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte3,           // 19
    MSEHasFallData,            // 140
    MSEMovementFlags2,         // 28
    MSEHasFallDirection,       // 136
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte1,  // 57
    MSEMovementFlags,          // 24
    MSEGuidByte2,              // 18
    MSEGuidByte3,              // 19
    MSEGuidByte5,              // 21
    MSEGuidByte6,              // 22
    MSEGuidByte0,              // 16
    MSEGuidByte4,              // 20
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEMovementForcesId,            // 156
    MSETransportTime3,         // 96
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportTime2,         // 88
    MSETransportPositionX,     // 64
    MSETransportSeat,          // 80
    MSETransportOrientation,   // 76
    MSETransportPositionY,     // 68
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte2,     // 58
    MSETransportPositionZ,     // 72
    MSETransportGuidByte6,     // 62
    MSETransportTime,          // 84
    MSESplineElevation,        // 144
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSETimestamp,              // 32
    MSEOrientation,            // 48
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementSetFacing[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEHasGuidByte4,           // 20
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 149
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte6,           // 22
    MSEHasTimestamp,           // 32
    MSEHasFallData,            // 140
    MSEHasOrientation,         // 48
    MSEHasGuidByte0,           // 16
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte1,           // 17
    MSEZeroBit,                // 172
    MSEHasPitch,               // 112
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 148
    MSEHasGuidByte3,           // 19
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte5,           // 21
    MSEHasUnkTime,             // 168
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportTime2,      // 92
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte0,  // 56
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte2,              // 18
    MSEMovementForcesId,            // 156
    MSEGuidByte1,              // 17
    MSEGuidByte6,              // 22
    MSEGuidByte7,              // 23
    MSEGuidByte0,              // 16
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEGuidByte4,              // 20
    MSETransportGuidByte4,     // 60
    MSETransportSeat,          // 80
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte2,     // 58
    MSETransportTime2,         // 88
    MSETransportOrientation,   // 76
    MSETransportPositionZ,     // 72
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSETransportTime,          // 84
    MSETransportPositionX,     // 64
    MSETransportPositionY,     // 68
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSESplineElevation,        // 144
    MSEPitch,                  // 112
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEOrientation,            // 48
    MSEEnd
};

MovementStatusElements const MovementSetPitch[] = // 5.4.7 18019
{
    /* Find MSEMovementForcesId */
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEHasTimestamp,           // 32
    MSEHasMovementFlags2,      // 28
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte4,           // 20
    MSEHasOrientation,         // 48
    MSEHasGuidByte7,           // 23
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte3,           // 19
    MSEHasFallData,            // 140
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte2,           // 18
    MSEHasUnkTime,             // 168
    MSEZeroBit,                // 149
    MSEHasGuidByte6,           // 22
    MSEHasPitch,               // 112
    MSEZeroBit,                // 172
    MSEHasGuidByte1,           // 17
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 148
    MSEMovementFlags2,         // 28
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime2,      // 92
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEGuidByte5,              // 21
    MSEGuidByte2,              // 18
    MSEGuidByte0,              // 16
    MSEGuidByte1,              // 17
    MSEGuidByte6,              // 22
    MSEGuidByte7,              // 23
    MSEGuidByte4,              // 20
    MSEGuidByte3,              // 19
    MSETransportGuidByte2,     // 58
    MSETransportTime2,         // 88
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte6,     // 62
    MSETransportTime,          // 84
    MSETransportGuidByte7,     // 63
    MSETransportPositionZ,     // 72
    MSETransportPositionY,     // 68
    MSETransportPositionX,     // 64
    MSETransportSeat,          // 80
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportGuidByte4,     // 60
    MSETransportOrientation,   // 76
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSEUnkTime,                // 168
    MSETimestamp,              // 32
    MSESplineElevation,        // 144
    MSEPitch,                  // 112
    MSEOrientation,            // 48
    MSEEnd
};

MovementStatusElements const MovementStartBackward[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSERemoveMovementForcesCount,           // 152
    MSEHasMovementFlags2,      // 28
    MSEHasPitch,               // 112
    MSEHasGuidByte1,           // 17
    MSEZeroBit,                // 149
    MSEHasGuidByte3,           // 19
    MSEHasOrientation,         // 48
    MSEHasFallData,            // 140
    MSEHasGuidByte7,           // 23
    MSEZeroBit,                // 172
    MSEHasGuidByte2,           // 18
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte4,           // 20
    MSEHasUnkTime,             // 168
    MSEZeroBit,                // 148
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte0,           // 16
    MSEHasTimestamp,           // 32
    MSEHasTransportData,       // 104
    MSEHasGuidByte6,           // 22
    MSEHasGuidByte5,           // 21
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte0,  // 56
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte7,              // 23
    MSEGuidByte5,              // 21
    MSEGuidByte6,              // 22
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte3,              // 19
    MSEMovementForcesId,            // 156
    MSEGuidByte1,              // 17
    MSEGuidByte4,              // 20
    MSETransportGuidByte7,     // 63
    MSETransportTime2,         // 88
    MSETransportGuidByte3,     // 59
    MSETransportSeat,          // 80
    MSETransportGuidByte4,     // 60
    MSETransportPositionY,     // 68
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte1,     // 57
    MSETransportPositionZ,     // 72
    MSETransportGuidByte0,     // 56
    MSETransportOrientation,   // 76
    MSETransportGuidByte6,     // 62
    MSETransportPositionX,     // 64
    MSETransportTime,          // 84
    MSETransportTime3,         // 96
    MSEPitch,                  // 112
    MSEFallTime,               // 116
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallVerticalSpeed,      // 120
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementStartForward[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEHasOrientation,         // 48
    MSEZeroBit,                // 148
    MSEHasGuidByte4,           // 20
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte3,           // 19
    MSEHasPitch,               // 112
    MSEHasTransportData,       // 104
    MSEHasGuidByte7,           // 23
    MSEHasSplineElevation,     // 144
    MSEHasFallData,            // 140
    MSEHasGuidByte2,           // 18
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 149
    MSEZeroBit,                // 172
    MSEHasGuidByte0,           // 16
    MSEHasUnkTime,             // 168
    MSEHasGuidByte5,           // 21
    MSEHasTimestamp,           // 32
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte1,           // 17
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte6,  // 62
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEGuidByte4,              // 20
    MSEGuidByte3,              // 19
    MSEGuidByte5,              // 21
    MSEGuidByte7,              // 23
    MSEGuidByte1,              // 17
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte6,              // 22
    MSEMovementForcesId,            // 156
    MSETransportGuidByte5,     // 61
    MSETransportPositionZ,     // 72
    MSETransportTime3,         // 96
    MSETransportPositionX,     // 64
    MSETransportGuidByte7,     // 63
    MSETransportPositionY,     // 68
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte4,     // 60
    MSETransportTime2,         // 88
    MSETransportTime,          // 84
    MSETransportGuidByte0,     // 56
    MSETransportOrientation,   // 76
    MSETransportSeat,          // 80
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte1,     // 57
    MSESplineElevation,        // 144
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementStartStrafeLeft[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEHasUnkTime,             // 168
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte2,           // 18
    MSEHasTimestamp,           // 32
    MSEZeroBit,                // 172
    MSEHasFallData,            // 140
    MSEHasGuidByte0,           // 16
    MSEHasOrientation,         // 48
    MSEHasGuidByte6,           // 22
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte5,           // 21
    MSEZeroBit,                // 148
    MSEHasGuidByte1,           // 17
    MSEHasTransportData,       // 104
    MSEHasGuidByte4,           // 20
    MSEHasMovementFlags2,      // 28
    MSEZeroBit,                // 149
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte7,           // 23
    MSEHasPitch,               // 112
    MSEHasGuidByte3,           // 19
    MSEMovementFlags,          // 24
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime3,      // 100
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte6,  // 62
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEGuidByte6,              // 22
    MSEGuidByte4,              // 20
    MSEGuidByte7,              // 23
    MSEGuidByte2,              // 18
    MSEGuidByte1,              // 17
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEMovementForcesId,            // 156
    MSEGuidByte0,              // 16
    MSETimestamp,              // 32
    MSETransportGuidByte7,     // 63
    MSETransportTime2,         // 88
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte3,     // 59
    MSETransportTime,          // 84
    MSETransportPositionY,     // 68
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte5,     // 61
    MSETransportPositionZ,     // 72
    MSETransportOrientation,   // 76
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte0,     // 56
    MSETransportPositionX,     // 64
    MSETransportGuidByte2,     // 58
    MSETransportTime3,         // 96
    MSETransportSeat,          // 80
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSEUnkTime,                // 168
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementStartStrafeRight[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEHasTransportData,       // 104
    MSEHasGuidByte0,           // 16
    MSEHasSplineElevation,     // 144
    MSEHasPitch,               // 112
    MSERemoveMovementForcesCount,           // 152
    MSEHasTimestamp,           // 32
    MSEHasGuidByte3,           // 19
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 172
    MSEZeroBit,                // 148
    MSEHasMovementFlags2,      // 28
    MSEHasOrientation,         // 48
    MSEZeroBit,                // 149
    MSEHasGuidByte7,           // 23
    MSEHasUnkTime,             // 168
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte2,           // 18
    MSEHasMovementFlags,       // 24
    MSEHasFallData,            // 140
    MSEHasGuidByte1,           // 17
    MSEMovementFlags2,         // 28
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime2,      // 92
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEMovementForcesId,            // 156
    MSEGuidByte4,              // 20
    MSEGuidByte0,              // 16
    MSEGuidByte6,              // 22
    MSEGuidByte2,              // 18
    MSEGuidByte7,              // 23
    MSEGuidByte1,              // 17
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportGuidByte4,     // 60
    MSETransportPositionZ,     // 72
    MSETransportTime2,         // 88
    MSETransportGuidByte6,     // 62
    MSETransportTime,          // 84
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte3,     // 59
    MSETransportPositionX,     // 64
    MSETransportOrientation,   // 76
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte5,     // 61
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSEOrientation,            // 48
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSETimestamp,              // 32
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementStartTurnLeft[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEHasGuidByte6,           // 22
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 149
    MSEHasUnkTime,             // 168
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte3,           // 19
    MSEHasOrientation,         // 48
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte1,           // 17
    MSEHasSplineElevation,     // 144
    MSEHasMovementFlags,       // 24
    MSEHasMovementFlags2,      // 28
    MSEHasFallData,            // 140
    MSEHasTransportData,       // 104
    MSEHasPitch,               // 112
    MSEHasGuidByte5,           // 21
    MSEZeroBit,                // 172
    MSEHasTimestamp,           // 32
    MSEZeroBit,                // 148
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte2,           // 18
    MSEHasFallDirection,       // 136
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte1,  // 57
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEMovementForcesId,            // 156
    MSEGuidByte6,              // 22
    MSEGuidByte3,              // 19
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEGuidByte0,              // 16
    MSEGuidByte4,              // 20
    MSEGuidByte5,              // 21
    MSEGuidByte2,              // 18
    MSETransportPositionX,     // 64
    MSETransportGuidByte0,     // 56
    MSETransportPositionY,     // 68
    MSETransportTime2,         // 88
    MSETransportGuidByte7,     // 63
    MSETransportTime3,         // 96
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte3,     // 59
    MSETransportPositionZ,     // 72
    MSETransportOrientation,   // 76
    MSETransportSeat,          // 80
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte2,     // 58
    MSETransportTime,          // 84
    MSEPitch,                  // 112
    MSEUnkTime,                // 168
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementStartTurnRight[] = // 5.4.7  18019
{
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEHasUnkTime,             // 168
    MSEHasFallData,            // 140
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte3,           // 19
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 172
    MSEHasGuidByte5,           // 21
    MSEHasOrientation,         // 48
    MSEHasGuidByte0,           // 16
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 148
    MSEHasTimestamp,           // 32
    MSEHasGuidByte6,           // 22
    MSEHasGuidByte7,           // 23
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 149
    MSEHasPitch,               // 112
    MSEHasGuidByte4,           // 20
    MSEHasSplineElevation,     // 144
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte1,  // 57
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEGuidByte2,              // 18
    MSEGuidByte5,              // 21
    MSEGuidByte4,              // 20
    MSEGuidByte6,              // 22
    MSEGuidByte1,              // 17
    MSEGuidByte3,              // 19
    MSEGuidByte7,              // 23
    MSEGuidByte0,              // 16
    MSEMovementForcesId,            // 156
    MSETimestamp,              // 32
    MSEOrientation,            // 48
    MSETransportTime3,         // 96
    MSETransportGuidByte6,     // 62
    MSETransportPositionY,     // 68
    MSETransportSeat,          // 80
    MSETransportGuidByte4,     // 60
    MSETransportTime,          // 84
    MSETransportPositionZ,     // 72
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte3,     // 59
    MSETransportOrientation,   // 76
    MSETransportPositionX,     // 64
    MSETransportGuidByte0,     // 56
    MSETransportTime2,         // 88
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte5,     // 61
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementStop[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEHasGuidByte1,           // 17
    MSEHasFallData,            // 140
    MSEHasMovementFlags2,      // 28
    MSEZeroBit,                // 172
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 149
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte6,           // 22
    MSEHasTimestamp,           // 32
    MSEHasGuidByte3,           // 19
    MSEZeroBit,                // 148
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte2,           // 18
    MSEHasUnkTime,             // 168
    MSEHasTransportData,       // 104
    MSEHasPitch,               // 112
    MSEHasGuidByte5,           // 21
    MSEHasOrientation,         // 48
    MSEHasGuidByte7,           // 23
    MSEMovementFlags,          // 24
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte0,  // 56
    MSEMovementFlags2,         // 28
    MSEHasFallDirection,       // 136
    MSEGuidByte1,              // 17
    MSEGuidByte6,              // 22
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEMovementForcesId,            // 156
    MSEGuidByte7,              // 23
    MSEGuidByte2,              // 18
    MSEGuidByte4,              // 20
    MSEGuidByte0,              // 16
    MSETransportGuidByte2,     // 58
    MSETransportTime,          // 84
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportGuidByte6,     // 62
    MSETransportTime2,         // 88
    MSETransportGuidByte3,     // 59
    MSETransportOrientation,   // 76
    MSETransportPositionX,     // 64
    MSETransportGuidByte4,     // 60
    MSETransportPositionZ,     // 72
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSETimestamp,              // 32
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementStopStrafe[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte0,           // 16
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte7,           // 23
    MSEHasFallData,            // 140
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 172
    MSEHasMovementFlags2,      // 28
    MSEZeroBit,                // 149
    MSEHasUnkTime,             // 168
    MSEZeroBit,                // 148
    MSEHasGuidByte5,           // 21
    MSEHasTransportData,       // 104
    MSEHasPitch,               // 112
    MSEHasOrientation,         // 48
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte3,           // 19
    MSEHasSplineElevation,     // 144
    MSEHasTimestamp,           // 32
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte1,           // 17
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte1,  // 57
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEHasFallDirection,       // 136
    MSEGuidByte6,              // 22
    MSEGuidByte7,              // 23
    MSEGuidByte4,              // 20
    MSEGuidByte1,              // 17
    MSEGuidByte5,              // 21
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte3,              // 19
    MSEMovementForcesId,            // 156
    MSETransportTime,          // 84
    MSETransportGuidByte2,     // 58
    MSETransportPositionX,     // 64
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte4,     // 60
    MSETransportPositionY,     // 68
    MSETransportGuidByte7,     // 63
    MSETransportSeat,          // 80
    MSETransportGuidByte6,     // 62
    MSETransportOrientation,   // 76
    MSETransportGuidByte1,     // 57
    MSETransportTime2,         // 88
    MSETransportPositionZ,     // 72
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte0,     // 56
    MSEPitch,                  // 112
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSETimestamp,              // 32
    MSEOrientation,            // 48
    MSESplineElevation,        // 144
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementStopTurn[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEHasGuidByte3,           // 19
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte6,           // 22
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasUnkTime,             // 168
    MSEHasTimestamp,           // 32
    MSEZeroBit,                // 149
    MSERemoveMovementForcesCount,           // 152
    MSEHasMovementFlags,       // 24
    MSEHasOrientation,         // 48
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 148
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte5,           // 21
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 172
    MSEHasPitch,               // 112
    MSEHasFallData,            // 140
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime3,      // 100
    MSEHasTransportTime2,      // 92
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEGuidByte1,              // 17
    MSEMovementForcesId,            // 156
    MSEGuidByte5,              // 21
    MSEGuidByte4,              // 20
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte7,              // 23
    MSEGuidByte6,              // 22
    MSEGuidByte3,              // 19
    MSETransportPositionX,     // 64
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportPositionZ,     // 72
    MSETransportPositionY,     // 68
    MSETransportSeat,          // 80
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte2,     // 58
    MSETransportTime2,         // 88
    MSETransportTime,          // 84
    MSETransportOrientation,   // 76
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte0,     // 56
    MSEUnkTime,                // 168
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSETimestamp,              // 32
    MSEEnd
};

MovementStatusElements const MovementStartAscend[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEHasGuidByte7,           // 23
    MSEHasMovementFlags,       // 24
    MSEZeroBit,                // 148
    MSEHasGuidByte2,           // 18
    MSEHasTimestamp,           // 32
    MSEHasGuidByte3,           // 19
    MSEZeroBit,                // 149
    MSEHasGuidByte4,           // 20
    MSEHasOrientation,         // 48
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte0,           // 16
    MSEHasSplineElevation,     // 144
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte5,           // 21
    MSEHasTransportData,       // 104
    MSEHasFallData,            // 140
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 172
    MSEHasUnkTime,             // 168
    MSEHasPitch,               // 112
    MSEMovementFlags2,         // 28
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte1,  // 57
    MSEMovementFlags,          // 24
    MSEHasFallDirection,       // 136
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte4,              // 20
    MSEMovementForcesId,            // 156
    MSEGuidByte3,              // 19
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEGuidByte5,              // 21
    MSEGuidByte6,              // 22
    MSEFallTime,               // 116
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSETransportPositionZ,     // 72
    MSETransportSeat,          // 80
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte4,     // 60
    MSETransportPositionX,     // 64
    MSETransportTime2,         // 88
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte3,     // 59
    MSETransportTime3,         // 96
    MSETransportOrientation,   // 76
    MSETransportPositionY,     // 68
    MSETransportTime,          // 84
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte2,     // 58
    MSESplineElevation,        // 144
    MSEUnkTime,                // 168
    MSETimestamp,              // 32
    MSEPitch,                  // 112
    MSEOrientation,            // 48
    MSEEnd
};

MovementStatusElements const MovementStartDescend[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEZeroBit,                // 149
    MSEHasGuidByte1,           // 17
    MSEHasPitch,               // 112
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 148
    MSEHasMovementFlags,       // 24
    MSEHasUnkTime,             // 168
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte7,           // 23
    MSEZeroBit,                // 172
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte3,           // 19
    MSEHasTimestamp,           // 32
    MSEHasOrientation,         // 48
    MSEHasTransportData,       // 104
    MSEHasSplineElevation,     // 144
    MSEHasFallData,            // 140
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte5,           // 21
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime3,      // 100
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte3,              // 19
    MSEGuidByte5,              // 21
    MSEGuidByte6,              // 22
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEGuidByte0,              // 16
    MSEGuidByte4,              // 20
    MSEMovementForcesId,            // 156
    MSEGuidByte2,              // 18
    MSETransportTime,          // 84
    MSETransportPositionX,     // 64
    MSETransportPositionY,     // 68
    MSETransportOrientation,   // 76
    MSETransportTime2,         // 88
    MSETransportPositionZ,     // 72
    MSETransportSeat,          // 80
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte4,     // 60
    MSETransportTime3,         // 96
    MSETransportGuidByte5,     // 61
    MSEUnkTime,                // 168
    MSESplineElevation,        // 144
    MSEFallVerticalSpeed,      // 120
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallTime,               // 116
    MSEPitch,                  // 112
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEEnd
};

MovementStatusElements const MovementStartSwim[] = // 5.4.7 18019
{
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEHasGuidByte3,           // 19
    MSEHasTimestamp,           // 32
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte4,           // 20
    MSEZeroBit,                // 149
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte0,           // 16
    MSEHasMovementFlags,       // 24
    MSEHasSplineElevation,     // 144
    MSEHasPitch,               // 112
    MSEHasFallData,            // 140
    MSEHasMovementFlags2,      // 28
    MSEHasTransportData,       // 104
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte6,           // 22
    MSEHasUnkTime,             // 168
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 148
    MSEHasOrientation,         // 48
    MSEZeroBit,                // 172
    MSEHasGuidByte1,           // 17
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte1,  // 57
    MSEMovementFlags,          // 24
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementForcesId,            // 156
    MSEGuidByte5,              // 21
    MSEGuidByte2,              // 18
    MSEGuidByte7,              // 23
    MSEGuidByte0,              // 16
    MSEGuidByte4,              // 20
    MSEGuidByte1,              // 17
    MSEGuidByte6,              // 22
    MSEGuidByte3,              // 19
    MSETransportGuidByte7,     // 63
    MSETransportPositionZ,     // 72
    MSETransportOrientation,   // 76
    MSETransportGuidByte2,     // 58
    MSETransportPositionX,     // 64
    MSETransportTime2,         // 88
    MSETransportTime,          // 84
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSETransportSeat,          // 80
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportPositionY,     // 68
    MSETransportGuidByte6,     // 62
    MSEOrientation,            // 48
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSESplineElevation,        // 144
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementStopSwim[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEZeroBit,                // 148
    MSEHasOrientation,         // 48
    MSEHasGuidByte3,           // 19
    MSEHasFallData,            // 140
    MSEHasTransportData,       // 104
    MSEHasGuidByte7,           // 23
    MSEZeroBit,                // 172
    MSEHasGuidByte6,           // 22
    MSEHasGuidByte1,           // 17
    MSEZeroBit,                // 149
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasUnkTime,             // 168
    MSEHasGuidByte5,           // 21
    MSEHasPitch,               // 112
    MSERemoveMovementForcesCount,           // 152
    MSEHasMovementFlags2,      // 28
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte2,           // 18
    MSEHasTimestamp,           // 32
    MSEMovementFlags,          // 24
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportTime3,      // 100
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementForcesId,            // 156
    MSEGuidByte4,              // 20
    MSEGuidByte1,              // 17
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEGuidByte2,              // 18
    MSEGuidByte0,              // 16
    MSEGuidByte6,              // 22
    MSEGuidByte7,              // 23
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportPositionY,     // 68
    MSETransportGuidByte2,     // 58
    MSETransportPositionX,     // 64
    MSETransportTime,          // 84
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte7,     // 63
    MSETransportPositionZ,     // 72
    MSETransportSeat,          // 80
    MSETransportGuidByte3,     // 59
    MSETransportOrientation,   // 76
    MSETransportTime2,         // 88
    MSEUnkTime,                // 168
    MSETimestamp,              // 32
    MSESplineElevation,        // 144
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementStopAscend[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEHasPitch,               // 112
    MSEHasGuidByte7,           // 23
    MSEHasMovementFlags2,      // 28
    MSEHasTimestamp,           // 32
    MSEHasFallData,            // 140
    MSEHasUnkTime,             // 168
    MSEHasGuidByte0,           // 16
    MSEZeroBit,                // 148
    MSEHasGuidByte2,           // 18
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 172
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte4,           // 20
    MSEZeroBit,                // 149
    MSEHasGuidByte1,           // 17
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte6,           // 22
    MSERemoveMovementForcesCount,           // 152
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte3,           // 19
    MSEHasOrientation,         // 48
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte4,  // 60
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte2,              // 18
    MSEGuidByte0,              // 16
    MSEGuidByte3,              // 19
    MSEGuidByte7,              // 23
    MSEGuidByte1,              // 17
    MSEGuidByte6,              // 22
    MSEGuidByte4,              // 20
    MSEMovementForcesId,            // 156
    MSEGuidByte5,              // 21
    MSESplineElevation,        // 144
    MSETransportOrientation,   // 76
    MSETransportPositionZ,     // 72
    MSETransportTime2,         // 88
    MSETransportSeat,          // 80
    MSETransportTime,          // 84
    MSETransportGuidByte6,     // 62
    MSETransportPositionY,     // 68
    MSETransportGuidByte5,     // 61
    MSETransportPositionX,     // 64
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte7,     // 63
    MSETransportTime3,         // 96
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte1,     // 57
    MSEOrientation,            // 48
    MSEPitch,                  // 112
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementStopPitch[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEHasFallData,            // 140
    MSEHasGuidByte6,           // 22
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte5,           // 21
    MSEZeroBit,                // 172
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte3,           // 19
    MSEHasMovementFlags2,      // 28
    MSEHasPitch,               // 112
    MSEZeroBit,                // 149
    MSEHasOrientation,         // 48
    MSEZeroBit,                // 148
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte2,           // 18
    MSEHasMovementFlags,       // 24
    MSEHasUnkTime,             // 168
    MSEHasTransportData,       // 104
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte1,           // 17
    MSEHasSplineElevation,     // 144
    MSEHasTimestamp,           // 32
    MSEMovementFlags,          // 24
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime3,      // 100
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEGuidByte3,              // 19
    MSEGuidByte4,              // 20
    MSEGuidByte5,              // 21
    MSEGuidByte0,              // 16
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEMovementForcesId,            // 156
    MSEGuidByte2,              // 18
    MSEGuidByte6,              // 22
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte3,     // 59
    MSETransportSeat,          // 80
    MSETransportGuidByte5,     // 61
    MSETransportTime,          // 84
    MSETransportPositionX,     // 64
    MSETransportGuidByte4,     // 60
    MSETransportPositionZ,     // 72
    MSETransportTime3,         // 96
    MSETransportPositionY,     // 68
    MSETransportGuidByte1,     // 57
    MSETransportOrientation,   // 76
    MSETransportTime2,         // 88
    MSETransportGuidByte0,     // 56
    MSEUnkTime,                // 168
    MSEFallTime,               // 116
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallVerticalSpeed,      // 120
    MSESplineElevation,        // 144
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementStartPitchDown[] = // 5.4.7 18019
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEZeroBit,                // 172
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 148
    MSEHasPitch,               // 112
    MSEHasTransportData,       // 104
    MSEHasGuidByte3,           // 19
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte4,           // 20
    MSEHasTimestamp,           // 32
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte7,           // 23
    MSEHasOrientation,         // 48
    MSEZeroBit,                // 149
    MSEHasFallData,            // 140
    MSEHasMovementFlags2,      // 28
    MSEHasUnkTime,             // 168
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte0,           // 16
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte6,           // 22
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte4,  // 60
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEGuidByte4,              // 20
    MSEGuidByte1,              // 17
    MSEGuidByte7,              // 23
    MSEMovementForcesId,            // 156
    MSEGuidByte6,              // 22
    MSEGuidByte3,              // 19
    MSEGuidByte5,              // 21
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSETransportGuidByte4,     // 60
    MSETransportSeat,          // 80
    MSETransportGuidByte5,     // 61
    MSETransportPositionZ,     // 72
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte1,     // 57
    MSETransportPositionX,     // 64
    MSETransportTime2,         // 88
    MSETransportGuidByte0,     // 56
    MSETransportOrientation,   // 76
    MSETransportTime,          // 84
    MSETransportGuidByte7,     // 63
    MSETransportPositionY,     // 68
    MSETransportGuidByte2,     // 58
    MSETransportTime3,         // 96
    MSETransportGuidByte3,     // 59
    MSEUnkTime,                // 168
    MSESplineElevation,        // 144
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementStartPitchUp[] = // 5.4.7 18019
{
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEZeroBit,                // 172
    MSEHasGuidByte5,           // 21
    MSEHasPitch,               // 112
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 148
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte1,           // 17
    MSEHasSplineElevation,     // 144
    MSEHasTimestamp,           // 32
    MSEHasMovementFlags,       // 24
    MSEHasMovementFlags2,      // 28
    MSEHasUnkTime,             // 168
    MSEHasFallData,            // 140
    MSEZeroBit,                // 149
    MSEHasGuidByte6,           // 22
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte3,           // 19
    MSEHasOrientation,         // 48
    MSEHasTransportData,       // 104
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte0,           // 16
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportTime2,      // 92
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte5,  // 61
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEHasFallDirection,       // 136
    MSEGuidByte4,              // 20
    MSEGuidByte7,              // 23
    MSEGuidByte3,              // 19
    MSEGuidByte2,              // 18
    MSEGuidByte5,              // 21
    MSEGuidByte1,              // 17
    MSEGuidByte0,              // 16
    MSEMovementForcesId,            // 156
    MSEGuidByte6,              // 22
    MSETransportPositionX,     // 64
    MSETransportGuidByte7,     // 63
    MSETransportTime2,         // 88
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportPositionZ,     // 72
    MSETransportGuidByte2,     // 58
    MSETransportTime3,         // 96
    MSETransportPositionY,     // 68
    MSETransportOrientation,   // 76
    MSETransportTime,          // 84
    MSETransportSeat,          // 80
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSESplineElevation,        // 144
    MSEPitch,                  // 112
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MoveChngTransport[]=
{
    MSEPositionY,
    MSEPositionZ,
    MSEPositionX,
    MSEHasSpline,
    MSEHasUnkTime,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEHasPitch,
    MSEHasTimestamp,
    MSEHasTransportData,
    MSEHasGuidByte4,
    MSEHasFallData,
    MSEHasGuidByte1,
    MSEHasOrientation,
    MSEHasGuidByte0,
    MSEHasGuidByte2,
    MSEZeroBit,
    MSEHasGuidByte5,
    MSEHasMovementFlags,
    MSEHasGuidByte3,
    MSEHasSplineElevation,
    MSEZeroBit,
    MSEHasMovementFlags2,
    MSERemoveMovementForcesCount,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte0,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEMovementFlags,
    MSEGuidByte5,
    MSEMovementForcesId,
    MSEGuidByte4,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte0,
    MSETransportOrientation,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportGuidByte1,
    MSETransportGuidByte7,
    MSETransportTime2,
    MSETransportGuidByte2,
    MSETransportSeat,
    MSETransportGuidByte0,
    MSETransportGuidByte5,
    MSETransportTime3,
    MSETransportPositionY,
    MSETransportPositionX,
    MSETransportTime,
    MSETransportGuidByte6,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEPitch,
    MSESplineElevation,
    MSEOrientation,
    MSEUnkTime,
    MSETimestamp,
    MSEEnd,
};

MovementStatusElements const MoveSplineDone[] =
{
    MSEPositionY,
    MSEPositionX,
    MSEPositionZ,
    MSEHasGuidByte6,
    MSEHasOrientation,
    MSEHasFallData,
    MSEHasTimestamp,
    MSEHasGuidByte2,
    MSEHasSplineElevation,
    MSEHasGuidByte4,
    MSEHasTransportData,
    MSEHasGuidByte3,
    MSEHasMovementFlags,
    MSEHasGuidByte0,
    MSEZeroBit,
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasPitch,
    MSEHasSpline,
    MSEHasMovementFlags2,
    MSEHasGuidByte7,

    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte0,

    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEMovementFlags,

    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte3,

    MSEFallVerticalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,

    MSEPitch,
    MSEOrientation,

    MSETransportGuidByte1,
    MSETransportTime3,
    MSETransportGuidByte7,
    MSETransportTime,
    MSETransportPositionY,
    MSETransportPositionX,
    MSETransportPositionZ,
    MSETransportSeat,
    MSETransportOrientation,
    MSETransportGuidByte0,
    MSETransportTime2,
    MSETransportGuidByte2,
    MSETransportGuidByte3,
    MSETransportGuidByte5,
    MSETransportGuidByte6,
    MSETransportGuidByte4,

    MSETimestamp,
    MSESplineElevation,
    MSEEnd,
};

MovementStatusElements const MoveNotActiveMover[] =
{
    MSEPositionZ,
    MSEPositionX,
    MSEPositionY,
    MSEHasMovementFlags2,
    MSEHasTransportData,
    MSEHasGuidByte6,
    MSEHasSplineElevation,
    MSEHasGuidByte3,
    MSEZeroBit,
    MSEHasTimestamp,
    MSEHasGuidByte0,
    MSEHasOrientation,
    MSEHasGuidByte5,
    MSEHasPitch,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasSpline,
    MSEHasGuidByte2,
    MSEHasFallData,
    MSEHasMovementFlags,

    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte3,

    MSEHasFallDirection,
    MSEMovementFlags,
    MSEMovementFlags2,

    MSEGuidByte1,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte3,

    MSEFallVerticalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,

    MSETransportTime3,
    MSETransportGuidByte1,
    MSETransportTime2,
    MSETransportOrientation,
    MSETransportGuidByte0,
    MSETransportSeat,
    MSETransportGuidByte4,
    MSETransportGuidByte6,
    MSETransportGuidByte3,
    MSETransportGuidByte5,
    MSETransportPositionY,
    MSETransportPositionX,
    MSETransportGuidByte2,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportTime,

    MSETimestamp,
    MSESplineElevation,
    MSEPitch,
    MSEOrientation,

    MSEEnd,
};

MovementStatusElements const DismissControlledVehicle[] =
{
    MSEPositionY,
    MSEPositionZ,
    MSEPositionX,
    MSEZeroBit,
    MSEHasUnkTime,
    MSEHasMovementFlags,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasTimestamp,
    MSERemoveMovementForcesCount,
    MSEHasPitch,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSEHasGuidByte0,
    MSEHasSplineElevation,
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte6,
    MSEHasSpline,
    MSEHasTransportData,
    MSEHasGuidByte1,
    MSEHasFallData,
    MSEHasOrientation,
    MSEHasMovementFlags2,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte4,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte1,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEMovementFlags,
    MSEGuidByte5,
    MSEMovementForcesId,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte4,
    MSEGuidByte1,
    MSETransportGuidByte1,
    MSETransportGuidByte5,
    MSETransportTime3,
    MSETransportGuidByte0,
    MSETransportPositionX,
    MSETransportOrientation,
    MSETransportGuidByte3,
    MSETransportGuidByte2,
    MSETransportTime2,
    MSETransportTime,
    MSETransportGuidByte4,
    MSETransportPositionY,
    MSETransportPositionZ,
    MSETransportGuidByte6,
    MSETransportSeat,
    MSETransportGuidByte7,
    MSETimestamp,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEPitch,
    MSEUnkTime,
    MSEOrientation,
    MSESplineElevation,
    MSEEnd,
};

MovementStatusElements const MoveUpdateTeleport[] =
{
    MSEPositionZ,
    MSEPositionY,
    MSEPositionX,
    MSEHasOrientation,

    MSEHasSpline,
    MSEHasMovementFlags,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte6,
    MSEHasFallData,
    MSEHasGuidByte0,
    MSEHasTransportData,
    MSEHasGuidByte5,

    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte0,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte2,

    MSEZeroBit,

    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasPitch,
    MSEHasMovementFlags2,
    MSEHasTimestamp,

    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEHasSplineElevation,
    MSEMovementFlags,
    MSEHasGuidByte1,

    MSEGuidByte7,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportOrientation,
    MSETransportTime3,
    MSETransportGuidByte1,
    MSETransportTime2,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportGuidByte0,
    MSETransportGuidByte6,
    MSETransportGuidByte5,
    MSETransportGuidByte2,
    MSETransportSeat,
    MSETransportTime,
    MSETransportPositionY,
    MSETransportPositionX,

    MSEGuidByte6,
    MSEPitch,
    MSESplineElevation,
    MSEOrientation,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte1,

    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,

    MSEGuidByte5,
    MSEGuidByte4,
    MSETimestamp,
    MSEGuidByte0,

    MSEEnd,
};

MovementStatusElements const MovementSetRunMode[] =
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEHasGuidByte1,           // 17
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 149
    MSEHasTransportData,       // 104
    MSEZeroBit,                // 148
    MSEHasGuidByte5,           // 21
    MSEHasGuidByte6,           // 22
    MSEHasMovementFlags,       // 24
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte0,           // 16
    MSEHasTimestamp,           // 32
    MSERemoveMovementForcesCount,           // 152
    MSEHasPitch,               // 112
    MSEHasUnkTime,             // 168
    MSEZeroBit,                // 172
    MSEHasOrientation,         // 48
    MSEHasFallData,            // 140
    MSEHasGuidByte3,           // 19
    MSEHasSplineElevation,     // 144
    MSEHasGuidByte4,           // 20
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime2,      // 92
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEHasFallDirection,       // 136
    MSEGuidByte1,              // 17
    MSEGuidByte2,              // 18
    MSEGuidByte3,              // 19
    MSEGuidByte5,              // 21
    MSEGuidByte7,              // 23
    MSEGuidByte6,              // 22
    MSECounter,                // 156
    MSEGuidByte4,              // 20
    MSEGuidByte0,              // 16
    MSEPitch,                  // 112
    MSETimestamp,              // 32
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte5,     // 61
    MSETransportOrientation,   // 76
    MSETransportGuidByte2,     // 58
    MSETransportPositionY,     // 68
    MSETransportPositionX,     // 64
    MSETransportGuidByte1,     // 57
    MSETransportTime,          // 84
    MSETransportTime3,         // 96
    MSETransportPositionZ,     // 72
    MSETransportTime2,         // 88
    MSETransportSeat,          // 80
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte4,     // 60
    MSETransportGuidByte6,     // 62
    MSEFallSinAngle,           // 128
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallVerticalSpeed,      // 120
    MSEFallTime,               // 116
    MSEUnkTime,                // 168
    MSEOrientation,            // 48
    MSESplineElevation,        // 144
    MSEEnd
};

MovementStatusElements const MovementSetWalkMode[] =
{
    MSEPositionZ,              // 44
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEHasTimestamp,           // 32
    MSEHasGuidByte5,           // 21
    MSEHasMovementFlags,       // 24
    MSEZeroBit,                // 148
    MSEHasGuidByte0,           // 16
    MSEZeroBit,                // 172
    MSEHasPitch,               // 112
    MSEHasTransportData,       // 104
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte4,           // 20
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte3,           // 19
    MSEHasOrientation,         // 48
    MSEHasGuidByte1,           // 17
    MSEHasUnkTime,             // 168
    MSEHasGuidByte6,           // 22
    MSEHasSplineElevation,     // 144
    MSEZeroBit,                // 149
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte2,           // 18
    MSEHasFallData,            // 140
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportTime3,      // 100
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSEGuidByte0,              // 16
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEGuidByte2,              // 18
    MSEGuidByte1,              // 17
    MSEGuidByte4,              // 20
    MSECounter,                // 156
    MSEGuidByte6,              // 22
    MSEGuidByte7,              // 23
    MSEFallVerticalSpeed,      // 120
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallTime,               // 116
    MSEPitch,                  // 112
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte7,     // 63
    MSETransportPositionZ,     // 72
    MSETransportGuidByte2,     // 58
    MSETransportGuidByte4,     // 60
    MSETransportPositionX,     // 64
    MSETransportGuidByte1,     // 57
    MSETransportTime3,         // 96
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportGuidByte6,     // 62
    MSETransportTime,          // 84
    MSETransportGuidByte3,     // 59
    MSETransportTime2,         // 88
    MSETransportOrientation,   // 76
    MSESplineElevation,        // 144
    MSETimestamp,              // 32
    MSEOrientation,            // 48
    MSEUnkTime,                // 168
    MSEEnd
};

MovementStatusElements const MovementSetCanFly[] =
{
    MSEPositionY,              // 40
    MSEPositionX,              // 36
    MSEPositionZ,              // 44
    MSEHasFallData,            // 140
    MSEHasGuidByte0,           // 16
    MSEHasMovementFlags2,      // 28
    MSEHasGuidByte6,           // 22
    MSEHasMovementFlags,       // 24
    MSEZeroBit,                // 172
    MSEHasGuidByte2,           // 18
    MSEHasOrientation,         // 48
    MSEHasUnkTime,             // 168
    MSEHasSplineElevation,     // 144
    MSEHasTransportData,       // 104
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte5,           // 21
    MSEHasPitch,               // 112
    MSEZeroBit,                // 148
    MSEHasGuidByte4,           // 20
    MSEHasTimestamp,           // 32
    MSERemoveMovementForcesCount,           // 152
    MSEZeroBit,                // 149
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte3,           // 19
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportTime2,      // 92
    MSEHasFallDirection,       // 136
    MSEMovementFlags,          // 24
    MSEMovementFlags2,         // 28
    MSECounter,                // 156
    MSEGuidByte4,              // 20
    MSEGuidByte2,              // 18
    MSEGuidByte6,              // 22
    MSEGuidByte3,              // 19
    MSEGuidByte0,              // 16
    MSEGuidByte7,              // 23
    MSEGuidByte1,              // 17
    MSEGuidByte5,              // 21
    MSEUnkTime,                // 168
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte4,     // 60
    MSETransportTime,          // 84
    MSETransportGuidByte3,     // 59
    MSETransportGuidByte6,     // 62
    MSETransportOrientation,   // 76
    MSETransportGuidByte0,     // 56
    MSETransportPositionZ,     // 72
    MSETransportPositionX,     // 64
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportTime2,         // 88
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte2,     // 58
    MSETransportTime3,         // 96
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallSinAngle,           // 128
    MSEFallTime,               // 116
    MSEFallVerticalSpeed,      // 120
    MSESplineElevation,        // 144
    MSEOrientation,            // 48
    MSETimestamp,              // 32
    MSEPitch,                  // 112
    MSEEnd
};

MovementStatusElements const MovementSetCanTransitionBetweenSwimAndFlyAck[] =
{
    MSEPositionZ,
    MSEPositionY,
    MSECounter,
    MSEPositionX,
    MSEHasGuidByte3,
    MSEHasOrientation,
    MSEHasTransportData,
    MSEHasMovementFlags,
    MSEHasMovementFlags2,
    MSEHasSplineElevation,
    MSEHasTimestamp,
    MSEHasSpline,
    MSEZeroBit,
    MSEHasFallData,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasPitch,
    MSEHasGuidByte7,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte2,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte1,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte6,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSETransportPositionY,
    MSETransportPositionZ,
    MSETransportGuidByte5,
    MSETransportGuidByte1,
    MSETransportGuidByte4,
    MSETransportTime,
    MSETransportSeat,
    MSETransportGuidByte3,
    MSETransportGuidByte0,
    MSETransportGuidByte2,
    MSETransportGuidByte7,
    MSETransportPositionX,
    MSETransportTime2,
    MSETransportGuidByte6,
    MSETransportOrientation,
    MSETransportTime3,
    MSEPitch,
    MSEOrientation,
    MSETimestamp,
    MSESplineElevation,
    MSEEnd,
};

MovementStatusElements const MovementUpdateSwimSpeed[] =
{
    MSEHasMovementFlags,
    MSEHasGuidByte2,
    MSEMovementFlags,
    MSEHasGuidByte3,
    MSEZeroBit,
    MSEHasGuidByte7,
    MSEHasFallDirection,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte7,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasOrientation,
    MSEHasGuidByte1,
    MSETransportGuidByte2,
    MSETransportPositionX,
    MSETransportGuidByte6,
    MSETransportOrientation,
    MSETransportGuidByte0,
    MSETransportGuidByte4,
    MSETransportPositionY,
    MSETransportTime,
    MSETransportGuidByte1,
    MSETransportGuidByte7,
    MSETransportPositionZ,
    MSETransportSeat,
    MSETransportGuidByte3,
    MSETransportGuidByte5,
    MSEPositionX,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEGuidByte7,
    MSETimestamp,
    MSEPositionY,
    MSEPositionZ,
    MSEExtraElement,
    MSEGuidByte4,
    MSEOrientation,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementUpdateRunSpeed[] =
{
    MSEPositionZ,
    MSEPositionX,
    MSEPositionY,
    MSEExtraElement,
    MSEHasGuidByte6,
    MSEHasMovementFlags2,
    MSEHasPitch,
    MSEHasGuidByte2,
    MSEHasGuidByte5,
    MSEHasSplineElevation,
    MSEHasSpline,
    MSEHasMovementFlags,
    MSEHasTimestamp,
    MSEHasGuidByte1,
    MSEMovementFlags2,
    MSEHasGuidByte3,
    MSEMovementFlags,
    MSEHasGuidByte7,
    MSEHasGuidByte0,
    MSEHasOrientation,
    MSEHasTransportData,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte0,
    MSEHasFallData,
    MSEHasFallDirection,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSETransportGuidByte4,
    MSETransportGuidByte5,
    MSETransportPositionX,
    MSETransportOrientation,
    MSETransportGuidByte1,
    MSETransportGuidByte0,
    MSETransportGuidByte6,
    MSETransportTime,
    MSETransportGuidByte7,
    MSETransportSeat,
    MSETransportTime2,
    MSETransportPositionY,
    MSETransportGuidByte3,
    MSETransportGuidByte2,
    MSETransportTime3,
    MSETransportPositionZ,
    MSETimestamp,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEPitch,
    MSEGuidByte6,
    MSESplineElevation,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEOrientation,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MovementUpdateFlightSpeed[] =
{
    MSEPositionY,
    MSEExtraElement,
    MSEPositionX,
    MSEPositionZ,
    MSEHasMovementFlags,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEMovementFlags,
    MSEHasGuidByte2,
    MSEHasOrientation,
    MSEHasGuidByte7,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte7,
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasFallDirection,
    MSEHasGuidByte0,
    MSEPitch,
    MSEGuidByte0,
    MSEGuidByte5,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportGuidByte5,
    MSETransportGuidByte4,
    MSETransportGuidByte6,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportGuidByte0,
    MSETransportGuidByte2,
    MSETransportGuidByte3,
    MSETransportPositionY,
    MSETransportGuidByte1,
    MSETransportSeat,
    MSETransportPositionX,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEGuidByte1,
    MSEGuidByte6,
    MSETimestamp,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte4,
    MSEOrientation,
    MSEGuidByte3,
    MSEEnd,
};

MovementStatusElements const MovementUpdateCollisionHeight[] =
{
    MSEPositionZ,
    MSEExtraElement,
    MSEPositionX,
    MSEPositionY,
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte2,
    MSEHasGuidByte3,
    MSEHasOrientation,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEZeroBit,
    MSEHasGuidByte2,
    MSEHasMovementFlags,
    MSEHasFallDirection,
    MSEHasGuidByte1,
    MSEMovementFlags,
    MSEGuidByte3,
    MSETransportGuidByte7,
    MSETransportTime,
    MSETransportGuidByte4,
    MSETransportGuidByte5,
    MSETransportOrientation,
    MSETransportPositionX,
    MSETransportGuidByte6,
    MSETransportGuidByte0,
    MSETransportPositionY,
    MSETransportGuidByte2,
    MSETransportPositionZ,
    MSETransportGuidByte3,
    MSETransportGuidByte1,
    MSETransportSeat,
    MSEPitch,
    MSEGuidByte6,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEGuidByte7,
    MSEOrientation,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte4,
    MSETimestamp,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MovementForceRunSpeedChangeAck[] =
{
    MSECounter,
    MSEPositionX,
    MSEExtraElement,
    MSEPositionZ,
    MSEPositionY,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEHasOrientation,
    MSEHasFallData,
    MSEHasGuidByte0,
    MSEHasSpline,
    MSEHasTransportData,
    MSEHasTimestamp,
    MSEHasMovementFlags2,
    MSEHasGuidByte6,
    MSEZeroBit,
    MSEHasSplineElevation,
    MSEHasPitch,
    MSEHasGuidByte5,
    MSEHasMovementFlags,
    MSEHasGuidByte3,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEMovementFlags,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte0,
    MSETransportPositionZ,
    MSETransportGuidByte6,
    MSETransportGuidByte1,
    MSETransportPositionY,
    MSETransportGuidByte0,
    MSETransportGuidByte5,
    MSETransportTime2,
    MSETransportPositionX,
    MSETransportTime,
    MSETransportGuidByte7,
    MSETransportOrientation,
    MSETransportGuidByte3,
    MSETransportTime3,
    MSETransportGuidByte2,
    MSETransportSeat,
    MSETransportGuidByte4,
    MSEFallVerticalSpeed,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallTime,
    MSESplineElevation,
    MSEPitch,
    MSETimestamp,
    MSEOrientation,
    MSEEnd,
};

MovementStatusElements const MovementSetCollisionHeightAck[] =
{
    MSEPositionX,
    MSEGenericDword0,
    MSEGenericDword1,
    MSECounter,
    MSEPositionZ,
    MSEPositionY,
    MSEHasUnkTime,
    MSEZeroBit,
    MSEHasMovementFlags,
    MSEZeroBit,
    MSEHasFallData,
    MSEHasTimestamp,
    MSEHasGuidByte4,
    MSEHasPitch,
    MSEHasSpline,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasSplineElevation,
    MSEHasGuidByte3,
    MSEHasOrientation,
    MSEHasGuidByte2,
    MSERemoveMovementForcesCount,
    MSEGeneric2bits0,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasMovementFlags2,
    MSEHasTransportData,
    MSEHasGuidByte7,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte4,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEMovementForcesId,
    MSEGuidByte4,
    MSEGuidByte0,
    MSETransportGuidByte1,
    MSETransportTime3,
    MSETransportPositionZ,
    MSETransportPositionY,
    MSETransportSeat,
    MSETransportGuidByte7,
    MSETransportGuidByte6,
    MSETransportGuidByte3,
    MSETransportOrientation,
    MSETransportPositionX,
    MSETransportGuidByte5,
    MSETransportTime2,
    MSETransportGuidByte0,
    MSETransportGuidByte4,
    MSETransportTime,
    MSETransportGuidByte2,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSETimestamp,
    MSESplineElevation,
    MSEOrientation,
    MSEUnkTime,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementForceFlightSpeedChangeAck[] =
{
    MSECounter,
    MSEPositionZ,
    MSEPositionX,
    MSEPositionY,
    MSEExtraElement,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEZeroBit,
    MSEHasMovementFlags,
    MSEHasSplineElevation,
    MSEHasGuidByte3,
    MSEHasMovementFlags2,
    MSEHasGuidByte7,
    MSEHasTransportData,
    MSEHasGuidByte0,
    MSEHasFallData,
    MSEHasTimestamp,
    MSEHasPitch,
    MSEHasSpline,
    MSEHasOrientation,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte0,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte4,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEMovementFlags,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte4,
    MSETransportGuidByte6,
    MSETransportGuidByte0,
    MSETransportSeat,
    MSETransportTime2,
    MSETransportOrientation,
    MSETransportGuidByte1,
    MSETransportGuidByte5,
    MSETransportTime3,
    MSETransportGuidByte2,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportPositionX,
    MSETransportTime,
    MSETransportPositionY,
    MSETimestamp,
    MSESplineElevation,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEPitch,
    MSEOrientation,
    MSEEnd,
};

MovementStatusElements const MovementSetCanFlyAck[] =
{
    MSEPositionY,
    MSEPositionX,
    MSEPositionZ,
    MSECounter,
    MSEHasPitch,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEHasUnkTime,
    MSEHasOrientation,
    MSEHasGuidByte6,
    MSEHasMovementFlags,
    MSEHasTimestamp,
    MSEHasGuidByte0,
    MSEHasMovementFlags2,
    MSERemoveMovementForcesCount,
    MSEZeroBit,
    MSEHasGuidByte4,
    MSEHasTransportData,
    MSEHasSpline,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasSplineElevation,
    MSEHasGuidByte7,
    MSEHasFallData,
    MSEZeroBit,
    MSEMovementFlags2,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte5,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte5,
    MSEMovementForcesId,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte6,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportPositionY,
    MSETransportGuidByte6,
    MSETransportTime3,
    MSETransportSeat,
    MSETransportGuidByte3,
    MSETransportGuidByte1,
    MSETransportGuidByte0,
    MSETransportGuidByte2,
    MSETransportTime2,
    MSETransportGuidByte5,
    MSETransportPositionX,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSETimestamp,
    MSEOrientation,
    MSESplineElevation,
    MSEPitch,
    MSEUnkTime,
    MSEEnd,
};

MovementStatusElements const MovementForceSwimSpeedChangeAck[] =
{
    MSEPositionX,
    MSECounter,
    MSEExtraElement,
    MSEPositionY,
    MSEPositionZ,
    MSEHasGuidByte4,
    MSEHasOrientation,
    MSEHasPitch,
    MSEHasTransportData,
    MSEHasMovementFlags,
    MSEHasMovementFlags2,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasFallData,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasTimestamp,
    MSEZeroBit,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasSplineElevation,
    MSEHasSpline,
    MSEHasGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte2,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte7,
    MSETransportGuidByte0,
    MSETransportTime3,
    MSETransportTime2,
    MSETransportPositionX,
    MSETransportGuidByte7,
    MSETransportSeat,
    MSETransportPositionY,
    MSETransportGuidByte5,
    MSETransportPositionZ,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportGuidByte2,
    MSETransportGuidByte6,
    MSETransportGuidByte3,
    MSETransportGuidByte1,
    MSETransportGuidByte4,
    MSEFallTime,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallVerticalSpeed,
    MSEOrientation,
    MSEPitch,
    MSESplineElevation,
    MSETimestamp,
    MSEEnd,
};

MovementStatusElements const MovementForceWalkSpeedChangeAck[] =
{
    MSEPositionZ,
    MSEPositionY,
    MSEExtraElement,
    MSEPositionX,
    MSECounter,
    MSEHasGuidByte0,
    MSEHasMovementFlags2,
    MSEHasTimestamp,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasMovementFlags,
    MSEHasFallData,
    MSEHasOrientation,
    MSEHasSpline,
    MSEZeroBit,
    MSEHasGuidByte3,
    MSEHasGuidByte1,
    MSEHasSplineElevation,
    MSEHasTransportData,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEHasPitch,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte3,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEFallVerticalSpeed,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallTime,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportGuidByte5,
    MSETransportPositionX,
    MSETransportGuidByte0,
    MSETransportTime,
    MSETransportTime3,
    MSETransportPositionY,
    MSETransportGuidByte2,
    MSETransportGuidByte4,
    MSETransportGuidByte1,
    MSETransportGuidByte3,
    MSETransportOrientation,
    MSETransportSeat,
    MSETransportGuidByte6,
    MSETransportTime2,
    MSESplineElevation,
    MSETimestamp,
    MSEOrientation,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementForceRunBackSpeedChangeAck[] =
{
    MSEExtraElement,
    MSECounter,
    MSEPositionX,
    MSEPositionZ,
    MSEPositionY,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasTimestamp,
    MSEHasTransportData,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasOrientation,
    MSEHasFallData,
    MSEHasMovementFlags2,
    MSEHasMovementFlags,
    MSEHasGuidByte7,
    MSEHasSpline,
    MSEHasPitch,
    MSEHasGuidByte6,
    MSEHasSplineElevation,
    MSEZeroBit,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte1,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte5,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte0,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSETransportGuidByte5,
    MSETransportSeat,
    MSETransportGuidByte6,
    MSETransportGuidByte1,
    MSETransportGuidByte0,
    MSETransportGuidByte2,
    MSETransportPositionY,
    MSETransportTime3,
    MSETransportPositionX,
    MSETransportOrientation,
    MSETransportGuidByte3,
    MSETransportTime,
    MSETransportTime2,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETimestamp,
    MSESplineElevation,
    MSEOrientation,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementUpdateRunBackSpeed[] =
{
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEZeroBit,
    MSEHasMovementFlags,
    MSEHasGuidByte5,
    MSEHasOrientation,
    MSEMovementFlags,
    MSEHasFallDirection,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte0,
    MSEHasGuidByte7,
    MSETransportPositionX,
    MSETransportGuidByte2,
    MSETransportGuidByte5,
    MSETransportGuidByte4,
    MSETransportGuidByte6,
    MSETransportGuidByte0,
    MSETransportGuidByte3,
    MSETransportPositionY,
    MSETransportGuidByte7,
    MSETransportPositionZ,
    MSETransportTime,
    MSETransportSeat,
    MSETransportGuidByte1,
    MSETransportOrientation,
    MSEGuidByte4,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSETimestamp,
    MSEGuidByte1,
    MSEOrientation,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte3,
    MSEPositionX,
    MSEPositionY,
    MSEPitch,
    MSEGuidByte7,
    MSEExtraElement,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEPositionZ,
    MSEEnd,
};

MovementStatusElements const MovementUpdateWalkSpeed[] =
{
    MSEHasOrientation,
    MSEZeroBit,
    MSEHasGuidByte3,
    MSEHasGuidByte2,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte5,
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasMovementFlags,
    MSEHasGuidByte6,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEHasGuidByte4,
    MSEPitch,
    MSETransportGuidByte6,
    MSETransportGuidByte0,
    MSETransportGuidByte4,
    MSETransportGuidByte2,
    MSETransportPositionX,
    MSETransportGuidByte7,
    MSETransportTime,
    MSETransportPositionZ,
    MSETransportSeat,
    MSETransportGuidByte5,
    MSETransportOrientation,
    MSETransportGuidByte1,
    MSETransportPositionY,
    MSETransportGuidByte3,
    MSEFallVerticalSpeed,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallTime,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte7,
    MSETimestamp,
    MSEOrientation,
    MSEPositionY,
    MSEGuidByte0,
    MSEPositionZ,
    MSEPositionX,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEExtraElement,
    MSEEnd,
};

MovementStatusElements const ForceMoveRootAck[] =
{
    MSEPositionY,
    MSEPositionX,
    MSEPositionZ,
    MSECounter,
    MSEZeroBit,
    MSEHasUnkTime,
    MSEHasGuidByte4,
    MSEHasOrientation,
    MSEHasFallData,
    MSEZeroBit,
    MSEHasGuidByte2,
    MSEHasTimestamp,
    MSEHasSpline,
    MSEHasGuidByte0,
    MSERemoveMovementForcesCount,
    MSEHasMovementFlags,
    MSEHasMovementFlags2,
    MSEHasSplineElevation,
    MSEHasGuidByte6,
    MSEHasPitch,
    MSEHasGuidByte7,
    MSEHasTransportData,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte0,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEMovementForcesId,
    MSETransportTime,
    MSETransportGuidByte3,
    MSETransportTime3,
    MSETransportGuidByte5,
    MSETransportGuidByte4,
    MSETransportPositionY,
    MSETransportPositionZ,
    MSETransportTime2,
    MSETransportPositionX,
    MSETransportGuidByte0,
    MSETransportGuidByte6,
    MSETransportGuidByte2,
    MSETransportGuidByte7,
    MSETransportSeat,
    MSETransportOrientation,
    MSETransportGuidByte1,
    MSEFallTime,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSETimestamp,
    MSESplineElevation,
    MSEUnkTime,
    MSEPitch,
    MSEOrientation,
    MSEEnd,
};

MovementStatusElements const ForceMoveUnrootAck[] =
{
    MSEPositionY,
    MSECounter,
    MSEPositionX,
    MSEPositionZ,
    MSEHasOrientation,
    MSEHasGuidByte0,
    MSERemoveMovementForcesCount,
    MSEHasTimestamp,
    MSEHasSplineElevation,
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEZeroBit,
    MSEHasPitch,
    MSEHasMovementFlags,
    MSEHasGuidByte1,
    MSEZeroBit,
    MSEHasFallData,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasSpline,
    MSEHasTransportData,
    MSEHasUnkTime,
    MSEHasMovementFlags2,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime3,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte4,
    MSEMovementForcesId,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEGuidByte6,
    MSETransportTime3,
    MSETransportGuidByte6,
    MSETransportPositionY,
    MSETransportGuidByte7,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportSeat,
    MSETransportPositionZ,
    MSETransportGuidByte4,
    MSETransportGuidByte0,
    MSETransportTime2,
    MSETransportGuidByte2,
    MSETransportGuidByte5,
    MSETransportGuidByte1,
    MSETransportGuidByte3,
    MSETransportPositionX,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEUnkTime,
    MSESplineElevation,
    MSEPitch,
    MSEOrientation,
    MSETimestamp,
    MSEEnd,
};

MovementStatusElements const MovementFallReset[] =
{
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEPositionX,              // 36
    MSEHasGuidByte1,           // 17
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 149
    MSEHasOrientation,         // 48
    MSEHasMovementFlags,       // 24
    MSEHasPitch,               // 112
    MSEHasMovementFlags2,      // 28
    MSERemoveMovementForcesCount,           // 152
    MSEHasTimestamp,           // 32
    MSEZeroBit,                // 148
    MSEZeroBit,                // 172
    MSEHasUnkTime,             // 168
    MSEHasGuidByte0,           // 16
    MSEHasGuidByte7,           // 23
    MSEHasGuidByte2,           // 18
    MSEHasGuidByte4,           // 20
    MSEHasSplineElevation,     // 144
    MSEHasFallData,            // 140
    MSEHasGuidByte5,           // 21
    MSEHasTransportData,       // 104
    MSEHasGuidByte3,           // 19
    MSEHasTransportTime3,      // 100
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte3,  // 59
    MSEHasFallDirection,       // 136
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte4,              // 20
    MSEGuidByte6,              // 22
    MSEGuidByte1,              // 17
    MSECounter,                // 156
    MSEGuidByte7,              // 23
    MSEGuidByte5,              // 21
    MSEGuidByte3,              // 19
    MSEGuidByte0,              // 16
    MSEGuidByte2,              // 18
    MSEFallTime,               // 116
    MSEFallSinAngle,           // 128
    MSEFallCosAngle,           // 124
    MSEFallHorizontalSpeed,    // 132
    MSEFallVerticalSpeed,      // 120
    MSETransportTime3,         // 96
    MSETransportGuidByte0,     // 56
    MSETransportOrientation,   // 76
    MSETransportGuidByte7,     // 63
    MSETransportGuidByte3,     // 59
    MSETransportPositionX,     // 64
    MSETransportTime2,         // 88
    MSETransportGuidByte2,     // 58
    MSETransportPositionZ,     // 72
    MSETransportGuidByte6,     // 62
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportGuidByte5,     // 61
    MSETransportGuidByte4,     // 60
    MSETransportTime,          // 84
    MSETransportGuidByte1,     // 57
    MSETimestamp,              // 32
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSEOrientation,            // 48
    MSEEnd
};

MovementStatusElements const MovementFeatherFallAck[] =
{
    MSEPositionZ,
    MSECounter,
    MSEPositionY,
    MSEPositionX,
    MSEZeroBit,
    MSEHasGuidByte3,
    MSEHasSplineElevation,
    MSEHasGuidByte1,
    MSEHasPitch,
    MSEHasGuidByte2,
    MSEHasSpline,
    MSEHasTimestamp,
    MSEHasGuidByte5,
    MSEHasFallData,
    MSEHasOrientation,
    MSEHasTransportData,
    MSEHasGuidByte7,
    MSEHasMovementFlags2,
    MSEHasGuidByte6,
    MSEHasMovementFlags,
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte0,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEMovementFlags,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte2,
    MSETransportTime3,
    MSETransportOrientation,
    MSETransportPositionZ,
    MSETransportGuidByte7,
    MSETransportGuidByte5,
    MSETransportGuidByte2,
    MSETransportPositionX,
    MSETransportGuidByte1,
    MSETransportGuidByte6,
    MSETransportTime2,
    MSETransportTime,
    MSETransportPositionY,
    MSETransportGuidByte4,
    MSETransportGuidByte0,
    MSETransportSeat,
    MSETransportGuidByte3,
    MSETimestamp,
    MSESplineElevation,
    MSEOrientation,
    MSEFallVerticalSpeed,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallTime,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementGravityDisableAck[] =
{
    MSEPositionZ,
    MSEPositionY,
    MSECounter,
    MSEPositionX,
    MSEHasGuidByte3,
    MSEHasTransportData,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasMovementFlags,
    MSEHasOrientation,
    MSEHasSpline,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasSplineElevation,
    MSEHasTimestamp,
    MSEHasPitch,
    MSEHasMovementFlags2,
    MSEZeroBit,
    MSEHasGuidByte2,
    MSEHasFallData,
    MSEHasGuidByte1,
    MSEHasTransportGuidByte2,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte4,
    MSEHasTransportTime2,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte6,
    MSETransportPositionZ,
    MSETransportGuidByte2,
    MSETransportGuidByte5,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportPositionY,
    MSETransportTime,
    MSETransportSeat,
    MSETransportGuidByte6,
    MSETransportGuidByte1,
    MSETransportGuidByte0,
    MSETransportOrientation,
    MSETransportTime2,
    MSETransportPositionX,
    MSETransportGuidByte7,
    MSETransportTime3,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEOrientation,
    MSESplineElevation,
    MSETimestamp,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementGravityEnableAck[] =
{
    MSEPositionZ,
    MSEPositionX,
    MSECounter,
    MSEPositionY,
    MSEHasSplineElevation,
    MSEHasMovementFlags2,
    MSEHasGuidByte6,
    MSEHasOrientation,
    MSEZeroBit,
    MSEHasGuidByte5,
    MSEHasSpline,
    MSEHasGuidByte3,
    MSEHasTransportData,
    MSEHasPitch,
    MSEHasGuidByte1,
    MSEHasTimestamp,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasMovementFlags,
    MSEHasFallData,
    MSEHasGuidByte0,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasFallDirection,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte6,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSETransportGuidByte1,
    MSETransportPositionX,
    MSETransportTime3,
    MSETransportOrientation,
    MSETransportGuidByte2,
    MSETransportGuidByte0,
    MSETransportGuidByte7,
    MSETransportGuidByte6,
    MSETransportSeat,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportTime2,
    MSETransportGuidByte5,
    MSETransportTime,
    MSETransportPositionY,
    MSESplineElevation,
    MSEOrientation,
    MSEPitch,
    MSETimestamp,
    MSEEnd,
};

MovementStatusElements const MovementHoverAck[] =
{
    MSECounter,
    MSEPositionZ,
    MSEPositionY,
    MSEPositionX,
    MSEHasGuidByte4,
    MSEHasTransportData,
    MSEHasGuidByte2,
    MSEHasTimestamp,
    MSEHasSpline,
    MSEHasMovementFlags,
    MSEHasGuidByte1,
    MSEHasPitch,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEZeroBit,
    MSEHasFallData,
    MSEHasMovementFlags2,
    MSEHasSplineElevation,
    MSEHasOrientation,
    MSEHasGuidByte3,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte7,
    MSEHasTransportTime2,
    MSEMovementFlags,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte0,
    MSETimestamp,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSEOrientation,
    MSETransportGuidByte1,
    MSETransportTime3,
    MSETransportGuidByte2,
    MSETransportGuidByte7,
    MSETransportGuidByte3,
    MSETransportGuidByte4,
    MSETransportSeat,
    MSETransportPositionX,
    MSETransportPositionZ,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportGuidByte0,
    MSETransportPositionY,
    MSETransportGuidByte5,
    MSETransportTime2,
    MSETransportGuidByte6,
    MSESplineElevation,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementKnockBackAck[] =
{
    MSEPositionX,              // 36
    MSEPositionY,              // 40
    MSEPositionZ,              // 44
    MSEExtraElement,
    MSEHasGuidByte2,           // 18
    MSEZeroBit,                // 149
    MSEHasGuidByte7,           // 23
    MSEHasOrientation,         // 48
    MSEHasGuidByte4,           // 20
    MSEHasTimestamp,           // 32
    MSEHasFallData,            // 140
    MSEZeroBit,                // 148
    MSEHasGuidByte0,           // 16
    MSEHasTransportData,       // 104
    MSEHasGuidByte5,           // 21
    MSERemoveMovementForcesCount,           // 152
    MSEHasGuidByte3,           // 19
    MSEHasMovementFlags,       // 24
    MSEHasMovementFlags2,      // 28
    MSEHasPitch,               // 112
    MSEHasGuidByte6,           // 22
    MSEZeroBit,                // 172
    MSEHasGuidByte1,           // 17
    MSEHasSplineElevation,     // 144
    MSEHasUnkTime,             // 168
    MSEHasFallDirection,       // 136
    MSEHasTransportGuidByte7,  // 63
    MSEHasTransportTime2,      // 92
    MSEHasTransportGuidByte6,  // 62
    MSEHasTransportGuidByte5,  // 61
    MSEHasTransportGuidByte0,  // 56
    MSEHasTransportGuidByte2,  // 58
    MSEHasTransportGuidByte4,  // 60
    MSEHasTransportGuidByte1,  // 57
    MSEHasTransportGuidByte3,  // 59
    MSEHasTransportTime3,      // 100
    MSEMovementFlags2,         // 28
    MSEMovementFlags,          // 24
    MSEGuidByte0,              // 16
    MSEGuidByte5,              // 21
    MSEMovementForcesId,            // 156
    MSEGuidByte6,              // 22
    MSEGuidByte1,              // 17
    MSEGuidByte4,              // 20
    MSEGuidByte2,              // 18
    MSEGuidByte3,              // 19
    MSEGuidByte7,              // 23
    MSEFallTime,               // 116
    MSEFallHorizontalSpeed,    // 132
    MSEFallCosAngle,           // 124
    MSEFallSinAngle,           // 128
    MSEFallVerticalSpeed,      // 120
    MSETransportTime,          // 84
    MSETransportOrientation,   // 76
    MSETransportGuidByte3,     // 59
    MSETransportPositionZ,     // 72
    MSETransportGuidByte2,     // 58
    MSETransportPositionX,     // 64
    MSETransportSeat,          // 80
    MSETransportPositionY,     // 68
    MSETransportTime3,         // 96
    MSETransportGuidByte1,     // 57
    MSETransportGuidByte6,     // 62
    MSETransportGuidByte7,     // 63
    MSETransportTime2,         // 88
    MSETransportGuidByte5,     // 61    
    MSETransportGuidByte0,     // 56
    MSETransportGuidByte4,     // 60
    MSEOrientation,            // 48
    MSEUnkTime,                // 168
    MSEPitch,                  // 112
    MSESplineElevation,        // 144
    MSETimestamp,              // 32
    MSEEnd
};

MovementStatusElements const MovementWaterWalkAck[] =
{
    MSEPositionY,
    MSEPositionZ,
    MSECounter,
    MSEPositionX,
    MSEHasTimestamp,
    MSEHasPitch,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasOrientation,
    MSEHasGuidByte1,
    MSEHasMovementFlags2,
    MSEHasGuidByte2,
    MSEHasMovementFlags,
    MSEHasGuidByte3,
    MSEHasTransportData,
    MSEHasGuidByte6,
    MSEHasFallData,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSEHasSplineElevation,
    MSEHasSpline,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte3,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte4,
    MSEMovementFlags,
    MSEHasFallDirection,
    MSEMovementFlags2,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte1,
    MSETransportPositionX,
    MSETransportGuidByte1,
    MSETransportTime3,
    MSETransportGuidByte0,
    MSETransportOrientation,
    MSETransportGuidByte7,
    MSETransportPositionY,
    MSETransportTime2,
    MSETransportTime,
    MSETransportGuidByte5,
    MSETransportSeat,
    MSETransportPositionZ,
    MSETransportGuidByte3,
    MSETransportGuidByte2,
    MSETransportGuidByte6,
    MSETransportGuidByte4,
    MSESplineElevation,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEOrientation,
    MSETimestamp,
    MSEPitch,
    MSEEnd,
};

MovementStatusElements const MovementUpdateKnockBack[] =
{
    MSEPositionX,
    MSEPositionY,
    MSEPositionZ,
    
    MSEHasTimestamp,
    MSEHasGuidByte4,
    MSEZeroBit,
    MSEHasPitch,
    MSEZeroBit,
    
    MSERemoveMovementForcesCount,
    MSEHasMovementFlags,
    MSEHasGuidByte3,
    MSEHasFallData,
    MSEZeroBit,
    MSEHasFallDirection,
    MSEHasMovementFlags2,
    MSEHasSplineElevation,
    MSEHasOrientation,
    MSEHasGuidByte1,
    MSEMovementFlags2,
    MSEHasUnkTime,
    MSEHasGuidByte7,
    MSEHasTransportData,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte0,
    MSEHasTransportTime3,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte5,
    MSEHasTransportGuidByte1,
    MSEHasGuidByte2,
    MSEMovementFlags,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte5,
    MSETransportPositionX,
    MSETransportGuidByte3,
    MSETransportTime,
    MSETransportTime3,
    MSETransportGuidByte5,
    MSETransportGuidByte4,
    MSETransportTime2,
    MSETransportGuidByte1,
    MSETransportPositionZ,
    MSETransportGuidByte2,
    MSETransportGuidByte0,
    MSETransportOrientation,
    MSETransportGuidByte7,
    MSETransportGuidByte6,
    MSETransportPositionY,
    MSETransportSeat,
    MSESplineElevation,
    MSEPitch,
    MSEMovementForcesId,
    MSEFallHorizontalSpeed,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallVerticalSpeed,
    MSEFallTime,
    MSEOrientation,
    MSEGuidByte5,
    MSEUnkTime,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte6,
    MSETimestamp,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetWalkSpeed[] =
{
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEExtraElement,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetRunSpeed[] =
{
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEGuidByte6,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEExtraElement,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetRunBackSpeed[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEGuidByte6,
    MSEExtraElement,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetSwimSpeed[] =
{
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte1,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEExtraElement,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetSwimBackSpeed[] =
{
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEGuidByte5,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEExtraElement,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetTurnRate[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEExtraElement,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetFlightSpeed[] =
{
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEExtraElement,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetFlightBackSpeed[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEHasGuidByte5,
    MSEExtraElement,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetPitchRate[] =
{
    MSEExtraElement,
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const MoveSetWalkSpeed[] = // 5.4.7 18019
{
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEExtraElement,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte5,
    MSECounter,
    MSEGuidByte2,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveSetRunSpeed[] = // 5.4.7 18019
{
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte0,
    MSECounter,
    MSEGuidByte5,
    MSEExtraElement,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MoveSetRunBackSpeed[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEGuidByte3,
    MSEExtraElement,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte6,
    MSECounter,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEEnd,
};

MovementStatusElements const MoveSetSwimSpeed[] = // 5.4.7 18019
{
    MSEExtraElement,
    MSECounter,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveSetSwimBackSpeed[] =
{
    MSECounter,
    MSEExtraElement,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveSetTurnRate[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEGuidByte6,
    MSEExtraElement,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte3,
    MSECounter,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEEnd,
};

MovementStatusElements const MoveSetFlightSpeed[] = //5.4.7 18019
{
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasGuidByte5,
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte3,
    MSECounter,
    MSEGuidByte5,
    MSEExtraElement,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MoveSetFlightBackSpeed[] =
{
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEGuidByte4,
    MSEExtraElement,
    MSEGuidByte0,
    MSECounter,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveSetPitchRate[] =
{
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasGuidByte4,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEExtraElement,
    MSECounter,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetWalkMode[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEGuidByte5,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetRunMode[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEEnd,
};

MovementStatusElements const SplineMoveGravityDisable[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte6,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveGravityEnable[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetHover[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveUnsetHover[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEEnd,
};

MovementStatusElements const SplineMoveStartSwim[] =
{
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveStopSwim[] =
{
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetFlying[] =
{
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const SplineMoveUnsetFlying[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte0,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetWaterWalk[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte4,
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte5,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetLandWalk[] =
{
    MSEHasGuidByte6,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEHasGuidByte2,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetFeatherFall[] =
{
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEHasGuidByte0,
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasGuidByte4,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const SplineMoveSetNormalFall[] =
{
    MSEHasGuidByte4,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte2,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte7,
    MSEGuidByte3,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const SplineMoveRoot[] =
{
    MSEHasGuidByte4,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte7,
    MSEGuidByte2,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const SplineMoveUnroot[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte4,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEGuidByte4,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte5,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MoveSetCanFly[] = // 5.4.7 18019
{
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEGuidByte0,
    MSEGuidByte5,
    MSECounter,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveUnsetCanFly[] = // 5.4.7 18019
{
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte2,
    MSEHasGuidByte4,
    MSEHasGuidByte7,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEGuidByte4,
    MSEGuidByte5,
    MSECounter,
    MSEGuidByte1,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const MoveSetHover[] =
{
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte7,
    MSEHasGuidByte4,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSECounter,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEEnd,
};

MovementStatusElements const MoveUnsetHover[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEGuidByte2,
    MSEGuidByte7,
    MSEGuidByte3,
    MSECounter,
    MSEGuidByte0,
    MSEGuidByte4,
    MSEGuidByte6,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEEnd,
};

MovementStatusElements const MoveWaterWalk[] =
{
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte5,
    MSEHasGuidByte0,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte0,
    MSEGuidByte7,
    MSEGuidByte5,
    MSEGuidByte3,
    MSEGuidByte4,
    MSECounter,
    MSEGuidByte1,
    MSEEnd,
};

MovementStatusElements const MoveLandWalk[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte3,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEGuidByte6,
    MSEGuidByte5,
    MSEGuidByte1,
    MSEGuidByte0,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte2,
    MSEGuidByte4,
    MSECounter,
    MSEEnd,
};

MovementStatusElements const MoveFeatherFall[] =
{
    MSECounter,
    MSEHasGuidByte0,
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte7,
    MSEHasGuidByte6,
    MSEHasGuidByte2,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte4,
    MSEGuidByte0,
    MSEGuidByte1,
    MSEGuidByte6,
    MSEGuidByte3,
    MSEGuidByte7,
    MSEEnd,
};

MovementStatusElements const MoveNormalFall[] =
{
    MSEHasGuidByte7,
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte0,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasGuidByte1,
    MSEHasGuidByte6,
    MSEGuidByte4,
    MSEGuidByte0,
    MSECounter,
    MSEGuidByte2,
    MSEGuidByte1,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte6,
    MSEEnd,
};

MovementStatusElements const MoveRoot[] =
{
    MSEHasGuidByte5,
    MSEHasGuidByte3,
    MSEHasGuidByte6,
    MSEHasGuidByte0,
    MSEHasGuidByte1,
    MSEHasGuidByte4,
    MSEHasGuidByte2,
    MSEHasGuidByte7,
    MSEGuidByte1,
    MSEGuidByte2,
    MSEGuidByte6,
    MSEGuidByte4,
    MSEGuidByte3,
    MSEGuidByte5,
    MSECounter,
    MSEGuidByte7,
    MSEGuidByte0,
    MSEEnd,
};

MovementStatusElements const MoveUnroot[] =
{
    MSEHasGuidByte0,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasGuidByte1,
    MSEHasGuidByte2,
    MSEHasGuidByte3,
    MSEHasGuidByte7,
    MSEHasGuidByte5,
    MSEGuidByte1,
    MSEGuidByte0,
    MSEGuidByte3,
    MSEGuidByte6,
    MSEGuidByte4,
    MSECounter,
    MSEGuidByte5,
    MSEGuidByte7,
    MSEGuidByte2,
    MSEEnd,
};

MovementStatusElements const ChangeSeatsOnControlledVehicle[] =
{
    MSEPositionY,
    MSEPositionX,
    MSEPositionZ,
    MSEExtraElement,
    MSEHasMovementFlags,
    MSEHasTransportData,
    MSEHasGuidByte2,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEExtraElement,
    MSEExtraElement,
    MSEHasOrientation,
    MSEZeroBit,
    MSEExtraElement,
    MSEHasGuidByte7,
    MSEExtraElement,
    MSEHasTimestamp,
    MSEHasSplineElevation,
    MSEHasGuidByte5,
    MSEExtraElement,
    MSEHasMovementFlags2,
    MSEHasPitch,
    MSEExtraElement,
    MSEHasGuidByte0,
    MSEExtraElement,
    MSEHasFallData,
    MSEHasGuidByte1,
    MSEHasSpline,
    MSEMovementFlags,
    MSEExtraElement,
    MSEHasGuidByte3,
    MSEHasTransportGuidByte3,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte2,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte4,
    MSEHasTransportGuidByte6,
    MSEMovementFlags2,
    MSEHasFallDirection,
    MSEExtraElement,
    MSEGuidByte7,
    MSEGuidByte5,
    MSEExtraElement,
    MSEExtraElement,
    MSEGuidByte6,
    MSEExtraElement,
    MSEExtraElement,
    MSEGuidByte3,
    MSEExtraElement,
    MSEGuidByte0,
    MSEExtraElement,
    MSEGuidByte4,
    MSEGuidByte1,
    MSEExtraElement,
    MSEGuidByte2,
    MSEPitch,
    MSEFallCosAngle,
    MSEFallSinAngle,
    MSEFallHorizontalSpeed,
    MSEFallTime,
    MSEFallVerticalSpeed,
    MSETransportGuidByte2,
    MSETransportTime2,
    MSETransportTime3,
    MSETransportGuidByte0,
    MSETransportTime,
    MSETransportSeat,
    MSETransportPositionX,
    MSETransportOrientation,
    MSETransportGuidByte7,
    MSETransportGuidByte4,
    MSETransportGuidByte3,
    MSETransportGuidByte5,
    MSETransportPositionZ,
    MSETransportGuidByte1,
    MSETransportGuidByte6,
    MSETransportPositionY,
    MSESplineElevation,
    MSEOrientation,
    MSETimestamp,
    MSEEnd,
};

MovementStatusElements const CastSpellEmbeddedMovement[] =
{
    MSEPositionZ,
    MSEPositionY,
    MSEPositionX,
    MSEHasFallData,
    MSEHasTimestamp,
    MSEHasOrientation,
    MSEZeroBit,
    MSEHasSpline,
    MSEHasGuidByte6,
    MSEHasGuidByte4,
    MSEHasMovementFlags2,
    MSEHasGuidByte3,
    MSEHasGuidByte5,
    MSEHasSplineElevation,
    MSEHasPitch,
    MSEHasGuidByte7,
    MSEHasTransportData,
    MSEHasGuidByte2,
    MSEHasMovementFlags,
    MSEHasGuidByte1,
    MSEHasGuidByte0,
    MSEHasTransportGuidByte6,
    MSEHasTransportGuidByte2,
    MSEHasTransportGuidByte5,
    MSEHasTransportTime2,
    MSEHasTransportGuidByte7,
    MSEHasTransportGuidByte4,
    MSEHasTransportTime3,
    MSEHasTransportGuidByte0,
    MSEHasTransportGuidByte1,
    MSEHasTransportGuidByte3,
    MSEMovementFlags2,
    MSEMovementFlags,
    MSEHasFallDirection,
    MSEGuidByte1,
    MSEGuidByte4,
    MSEGuidByte7,
    MSEGuidByte3,
    MSEGuidByte0,
    MSEGuidByte2,
    MSEGuidByte5,
    MSEGuidByte6,
    MSETransportSeat,
    MSETransportOrientation,
    MSETransportTime,
    MSETransportGuidByte6,
    MSETransportGuidByte5,
    MSETransportTime3,
    MSETransportPositionX,
    MSETransportGuidByte4,
    MSETransportPositionZ,
    MSETransportGuidByte2,
    MSETransportGuidByte0,
    MSETransportTime2,
    MSETransportGuidByte1,
    MSETransportGuidByte3,
    MSETransportPositionY,
    MSETransportGuidByte7,
    MSEOrientation,
    MSESplineElevation,
    MSEFallTime,
    MSEFallHorizontalSpeed,
    MSEFallSinAngle,
    MSEFallCosAngle,
    MSEFallVerticalSpeed,
    MSETimestamp,
    MSEPitch,
    MSEEnd,
};

void ExtraMovementStatusElement::ReadNextElement(ByteBuffer& packet)
{
    MovementStatusElements const element = _elements[_index++];

    switch (element)
    {
        case MSEHasGuidByte0:
        case MSEHasGuidByte1:
        case MSEHasGuidByte2:
        case MSEHasGuidByte3:
        case MSEHasGuidByte4:
        case MSEHasGuidByte5:
        case MSEHasGuidByte6:
        case MSEHasGuidByte7:
            Data.guid[element - MSEHasGuidByte0] = packet.ReadBit();
            break;
        case MSEGuidByte0:
        case MSEGuidByte1:
        case MSEGuidByte2:
        case MSEGuidByte3:
        case MSEGuidByte4:
        case MSEGuidByte5:
        case MSEGuidByte6:
        case MSEGuidByte7:
            packet.ReadByteSeq(Data.guid[element - MSEGuidByte0]);
            break;
        case MSEExtraFloat:
            packet >> Data.floatData;
            break;
        case MSEExtraInt8:
            packet >> Data.byteData;
            break;
        default:
            ASSERT(PrintInvalidSequenceElement(element, __FUNCTION__));
            break;
    }
}

void ExtraMovementStatusElement::WriteNextElement(ByteBuffer& packet)
{
    MovementStatusElements const element = _elements[_index++];

    switch (element)
    {
        case MSEHasGuidByte0:
        case MSEHasGuidByte1:
        case MSEHasGuidByte2:
        case MSEHasGuidByte3:
        case MSEHasGuidByte4:
        case MSEHasGuidByte5:
        case MSEHasGuidByte6:
        case MSEHasGuidByte7:
            packet.WriteBit(Data.guid[element - MSEHasGuidByte0]);
            break;
        case MSEGuidByte0:
        case MSEGuidByte1:
        case MSEGuidByte2:
        case MSEGuidByte3:
        case MSEGuidByte4:
        case MSEGuidByte5:
        case MSEGuidByte6:
        case MSEGuidByte7:
            packet.WriteByteSeq(Data.guid[element - MSEGuidByte0]);
            break;
        case MSEExtraFloat:
            packet << Data.floatData;
            break;
        case MSEExtraInt8:
            packet << Data.byteData;
            break;
        default:
            ASSERT(PrintInvalidSequenceElement(element, __FUNCTION__));
            break;
    }
}

bool PrintInvalidSequenceElement(MovementStatusElements const element, char const* function)
{
    sLog->outError(LOG_FILTER_OPCODES, "Incorrect sequence element %d detected at %s", element, function);
    return false;
}

void PacketSender::Send() const
{
    ASSERT(_broadcast != NULL_OPCODE || _self != NULL_OPCODE); 

    WorldPacket data;

    if (_broadcast != NULL_OPCODE)
    {
        data.Initialize(_broadcast, 12);
        _unit->WriteMovementInfo(data, _extraElements);
        _unit->SendMessageToSet(&data, false);
    }

    if (_self != NULL_OPCODE)
    {
        if (Player* player = _unit->ToPlayer())
        {
            if (_extraElements)
                _extraElements->ResetIndex();

            data.Initialize(_self, 12);
            player->WriteMovementInfo(data, _extraElements);
            player->SendDirectMessage(&data);
        }
    }

    /*bool isPlayerMovement = false;
    if (Player* player = _unit->ToPlayer())
    {
        isPlayerMovement = player->m_mover->GetTypeId() == TYPEID_PLAYER;
        if (isPlayerMovement && _selfOpcode != NULL_OPCODE)
        {
            WorldPacket data(_selfOpcode);
            _unit->WriteMovementInfo(data, _extraElements);
            player->SendDirectMessage(&data);
        }
    }

    if (_broadcast != NULL_OPCODE)
    {
        ///! Need to reset current extra element index before writing another packet
        if (_extraElements)
            _extraElements->ResetIndex();

        WorldPacket data(_broadcast);
        _unit->WriteMovementInfo(data, _extraElements);
        _unit->SendMessageToSet(&data, !isPlayerMovement);
    }*/
}

MovementStatusElements const* GetMovementStatusElementsSequence(uint16 opcode)
{
    switch (opcode)
    {
        case CMSG_MOVE_FALL_LAND:
            return MovementFallLand;
        case CMSG_MOVE_HEARTBEAT:
            return MovementHeartBeat;
        case CMSG_MOVE_JUMP:
            return MovementJump;
        case CMSG_MOVE_SET_FACING:
            return MovementSetFacing;
        case CMSG_MOVE_SET_PITCH:
            return MovementSetPitch;
        case CMSG_MOVE_START_ASCEND:
           return MovementStartAscend;
        case CMSG_MOVE_START_BACKWARD:
           return MovementStartBackward;
        case CMSG_MOVE_START_DESCEND:
            return MovementStartDescend;
        case CMSG_MOVE_START_FORWARD:
            return MovementStartForward;
        case CMSG_MOVE_START_PITCH_DOWN:
            return MovementStartPitchDown;
        case CMSG_MOVE_START_PITCH_UP:
            return MovementStartPitchUp;
        case CMSG_MOVE_START_STRAFE_LEFT:
            return MovementStartStrafeLeft;
        case CMSG_MOVE_START_STRAFE_RIGHT:
            return MovementStartStrafeRight;
        case CMSG_MOVE_START_SWIM:
            return MovementStartSwim;
        case CMSG_MOVE_START_TURN_LEFT:
            return MovementStartTurnLeft;
        case CMSG_MOVE_START_TURN_RIGHT:
            return MovementStartTurnRight;
        case CMSG_MOVE_STOP:
            return MovementStop;
        case CMSG_MOVE_STOP_ASCEND:
            return MovementStopAscend;
        case CMSG_MOVE_STOP_PITCH:
            return MovementStopPitch;
        case CMSG_MOVE_STOP_STRAFE:
            return MovementStopStrafe;
        case CMSG_MOVE_STOP_SWIM:
            return MovementStopSwim;
        case CMSG_MOVE_STOP_TURN:
            return MovementStopTurn;
        case SMSG_MOVE_UPDATE:
            return PlayerMove;
        case CMSG_MOVE_CHNG_TRANSPORT:
            return MoveChngTransport;
        //case CMSG_MOVE_SPLINE_DONE:
        //    return MoveSplineDone;
        //case CMSG_MOVE_NOT_ACTIVE_MOVER:
        //    return MoveNotActiveMover;
        case CMSG_DISMISS_CONTROLLED_VEHICLE:
            return DismissControlledVehicle;
        //case SMSG_MOVE_UPDATE_TELEPORT:
        //    return MoveUpdateTeleport;
        case CMSG_FORCE_MOVE_ROOT_ACK:
            return ForceMoveRootAck;
        case CMSG_FORCE_MOVE_UNROOT_ACK:
            return ForceMoveUnrootAck;
        //case CMSG_MOVE_FALL_RESET:
        //    return MovementFallReset;
        //case CMSG_MOVE_FEATHER_FALL_ACK:
        //    return MovementFeatherFallAck;
        //case CMSG_MOVE_FORCE_FLIGHT_SPEED_CHANGE_ACK:
        //    return MovementForceFlightSpeedChangeAck;
        //case CMSG_MOVE_FORCE_RUN_BACK_SPEED_CHANGE_ACK:
        //    return MovementForceRunBackSpeedChangeAck;
        //case CMSG_MOVE_FORCE_RUN_SPEED_CHANGE_ACK:
        //    return MovementForceRunSpeedChangeAck;
        //case CMSG_MOVE_FORCE_SWIM_SPEED_CHANGE_ACK:
        //    return MovementForceSwimSpeedChangeAck;
        //case CMSG_MOVE_FORCE_WALK_SPEED_CHANGE_ACK:
        //    return MovementForceWalkSpeedChangeAck;
        //case CMSG_MOVE_GRAVITY_DISABLE_ACK:
        //    return MovementGravityDisableAck;
        //case CMSG_MOVE_GRAVITY_ENABLE_ACK:
        //    return MovementGravityEnableAck;
        //case CMSG_MOVE_HOVER_ACK:
        //    return MovementHoverAck;
        case CMSG_MOVE_KNOCK_BACK_ACK:
            return MovementKnockBackAck;
        //case CMSG_MOVE_SET_CAN_FLY:
        //    return MovementSetCanFly;
        case CMSG_MOVE_SET_CAN_FLY_ACK:
            return MovementSetCanFlyAck;
        //case CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK:
        //    return MovementSetCanTransitionBetweenSwimAndFlyAck;
        case CMSG_MOVE_SET_COLLISION_HEIGHT_ACK:
            return MovementSetCollisionHeightAck;
        //case SMSG_MOVE_UPDATE_COLLISION_HEIGHT:
        //    return MovementUpdateCollisionHeight;
        //case CMSG_MOVE_WATER_WALK_ACK:
        //    return MovementWaterWalkAck;
        //case MSG_MOVE_SET_RUN_MODE:
        //    return MovementSetRunMode;
        //case MSG_MOVE_SET_WALK_MODE:
        //    return MovementSetWalkMode;
        //case SMSG_MOVE_UPDATE_FLIGHT_SPEED:
        //    return MovementUpdateFlightSpeed;
        //case SMSG_MOVE_UPDATE_RUN_SPEED:
        //    return MovementUpdateRunSpeed;
        case SMSG_MOVE_UPDATE_KNOCK_BACK:
            return MovementUpdateKnockBack;
        //case SMSG_MOVE_UPDATE_RUN_BACK_SPEED:
        //    return MovementUpdateRunBackSpeed;
        //case SMSG_MOVE_UPDATE_SWIM_SPEED:
        //    return MovementUpdateSwimSpeed;
        //case SMSG_MOVE_UPDATE_WALK_SPEED:
        //    return MovementUpdateWalkSpeed;
        case SMSG_SPLINE_MOVE_SET_WALK_SPEED:
            return SplineMoveSetWalkSpeed;
        case SMSG_SPLINE_MOVE_SET_RUN_SPEED:
            return SplineMoveSetRunSpeed;
        case SMSG_SPLINE_MOVE_SET_RUN_BACK_SPEED:
            return SplineMoveSetRunBackSpeed;
        case SMSG_SPLINE_MOVE_SET_SWIM_SPEED:
            return SplineMoveSetSwimSpeed;
        case SMSG_SPLINE_MOVE_SET_SWIM_BACK_SPEED:
            return SplineMoveSetSwimBackSpeed;
        case SMSG_SPLINE_MOVE_SET_TURN_RATE:
            return SplineMoveSetTurnRate;
        case SMSG_SPLINE_MOVE_SET_FLIGHT_SPEED:
            return SplineMoveSetFlightSpeed;
        case SMSG_SPLINE_MOVE_SET_FLIGHT_BACK_SPEED:
            return SplineMoveSetFlightBackSpeed;
        case SMSG_SPLINE_MOVE_SET_PITCH_RATE:
            return SplineMoveSetPitchRate;
        case SMSG_MOVE_SET_WALK_SPEED:
            return MoveSetWalkSpeed;
        case SMSG_MOVE_SET_RUN_SPEED:
            return MoveSetRunSpeed;
        case SMSG_MOVE_SET_RUN_BACK_SPEED:
            return MoveSetRunBackSpeed;
        case SMSG_MOVE_SET_SWIM_SPEED:
            return MoveSetSwimSpeed;
        case SMSG_MOVE_SET_SWIM_BACK_SPEED:
            return MoveSetSwimBackSpeed;
        case SMSG_MOVE_SET_TURN_RATE:
            return MoveSetTurnRate;
        case SMSG_MOVE_SET_FLIGHT_SPEED:
           return MoveSetFlightSpeed;
        case SMSG_MOVE_SET_FLIGHT_BACK_SPEED:
            return MoveSetFlightBackSpeed;
        case SMSG_MOVE_SET_PITCH_RATE:
            return MoveSetPitchRate;
        case SMSG_SPLINE_MOVE_SET_WALK_MODE:
            return SplineMoveSetWalkMode;
        case SMSG_SPLINE_MOVE_SET_RUN_MODE:
            return SplineMoveSetRunMode;
        //case SMSG_SPLINE_MOVE_GRAVITY_DISABLE:
        //    return SplineMoveGravityDisable;
        //case SMSG_SPLINE_MOVE_GRAVITY_ENABLE:
        //    return SplineMoveGravityEnable;
        case SMSG_SPLINE_MOVE_SET_HOVER:
            return SplineMoveSetHover;
        case SMSG_SPLINE_MOVE_UNSET_HOVER:
            return SplineMoveUnsetHover;
        case CMSG_MOVE_SET_FLY:
            return SetCanFly;
        //case SMSG_SPLINE_MOVE_START_SWIM:
        //    return SplineMoveStartSwim;
        //case SMSG_SPLINE_MOVE_STOP_SWIM:
        //    return SplineMoveStopSwim;
        case SMSG_SPLINE_MOVE_SET_FLYING:
            return SplineMoveSetFlying;
        case SMSG_SPLINE_MOVE_UNSET_FLYING:
            return SplineMoveUnsetFlying;
        case SMSG_SPLINE_MOVE_SET_WATER_WALK:
            return SplineMoveSetWaterWalk;
        case SMSG_SPLINE_MOVE_SET_LAND_WALK:
            return SplineMoveSetLandWalk;
        case SMSG_SPLINE_MOVE_SET_FEATHER_FALL:
            return SplineMoveSetFeatherFall;
        case SMSG_SPLINE_MOVE_SET_NORMAL_FALL:
            return SplineMoveSetNormalFall;
        case SMSG_SPLINE_MOVE_ROOT:
            return SplineMoveRoot;
        case SMSG_SPLINE_MOVE_UNROOT:
            return SplineMoveUnroot;
        case SMSG_MOVE_SET_CAN_FLY:
            return MoveSetCanFly;
        case SMSG_MOVE_UNSET_CAN_FLY:
            return MoveUnsetCanFly;
        case SMSG_MOVE_SET_HOVER:
            return MoveSetHover;
        case SMSG_MOVE_UNSET_HOVER:
            return MoveUnsetHover;
        case SMSG_MOVE_WATER_WALK:
            return MoveWaterWalk;
        case SMSG_MOVE_LAND_WALK:
            return MoveLandWalk;
        case SMSG_MOVE_FEATHER_FALL:
            return MoveFeatherFall;
        case SMSG_MOVE_NORMAL_FALL:
            return MoveNormalFall;
        case SMSG_MOVE_ROOT:
            return MoveRoot;
        case SMSG_MOVE_UNROOT:
            return MoveUnroot;
        //case CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE:
        //    return ChangeSeatsOnControlledVehicle;
        //case CMSG_CAST_SPELL:
        //case CMSG_PET_CAST_SPELL:
        //case CMSG_USE_ITEM:
        //    return CastSpellEmbeddedMovement;*/
        default:
            break;
    }

    return NULL;
}
