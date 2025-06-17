// =============================================================================
// Constants
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef CONST_H
#define CONST_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Constants
// =============================================================================

#define PI              3.1415926535897932      // pi
#define D2R             (PI/180.0)              // deg to rad
#define R2D             (180.0/PI)              // rad to deg
#define SC2RAD          3.1415926535898         // semi-circle to radian (GPS)
#define C_LIGHT         299792458.0             // speed of light

#define RE_WGS84        6378137.0               // Earth's semimajor axis (WGS84) (m)
#define RE_GLO          6378136.0               // Earth's semimajor axis (PZ90.02) (m)

#define FE_WGS84        (1.0/298.257223563)     // Earth's flattening (WGS84)

#define FREQ_L1         1575.420E6              // GPS/QZSS/SBAS L1
#define FREQ_L2         1227.600E6              // GPS/QZSS L2
#define FREQ_L5         1176.450E6              // GPS/QZSS/SBAS L5

#define FREQ_G1         1602.000E6              // GLONASS G1 base
#define FREQ_G2         1246.000E6              // GLONASS G2 base
#define DFREQ_G1        0.562500E6              // GLONASS G1 bias
#define DFREQ_G2        0.437500E6              // GLONASS G2 bias
#define FREQ_G3         1202.025E6              // GLONASS G3 (CDMA)
#define FREQ_G1A        1600.995E6              // GLONASS G1a (CDMA)
#define FREQ_G2A        1248.060E6              // GLONASS G2a (CDMA)

#define FREQ_E1         1575.420E6              // Galileo E1
#define FREQ_E5A        1176.450E6              // Galileo E5a
#define FREQ_E5B        1207.140E6              // Galileo E5b
#define FREQ_E5AB       1191.795E6              // Galileo E5a+b
#define FREQ_E6         1278.750E6              // Galileo E6

#define FREQ_L6         1278.750E6              // QZSS L6

#define FREQ_B1         1561.098E6              // BeiDou B1I
#define FREQ_B1C        1575.420E6              // BeiDou B1C
#define FREQ_B2A        1176.450E6              // BeiDou B2a
#define FREQ_B2B        1207.140E6              // BeiDou B2I/B2b
#define FREQ_B2AB       1191.795E6              // BeiDou B2a+b
#define FREQ_B3         1268.520E6              // BeiDou B3

#define FREQ_S          2492.028E6              // IRNSS S

#define NRCV            2                       // Number of receivers
#define NFREQ           2                       // Number of frequency index
#define NBAND           9                       // Number of frequency bands

#define SYS_GPS         1                       // GPS system flag
#define SYS_GLO         0                       // GLO system flag
#define SYS_GAL         1                       // GAL system flag
#define SYS_BDS         1                       // BDS system flag
#define SYS_QZS         0                       // QZS system flag
#define SYS_IRN         0                       // IRN system flag
#define SYS_SBS         0                       // SBS system flag

#define NSYS            (SYS_GPS + SYS_GLO + SYS_GAL + SYS_BDS + SYS_QZS + SYS_IRN + SYS_SBS)
                                                // Number of systems

#define STR_GPS         'G'                     // GPS system identifier
#define STR_GLO         'R'                     // GLO system identifier
#define STR_GAL         'E'                     // GAL system identifier
#define STR_BDS         'C'                     // BDS system identifier
#define STR_QZS         'J'                     // QZS system identifier
#define STR_IRN         'I'                     // IRN system identifier
#define STR_SBS         'S'                     // SBS system identifier

#define MIN_PRN_GPS     (SYS_GPS ?  1 : 0)
#define MAX_PRN_GPS     (SYS_GPS ? 32 : 0)
#define NSAT_GPS        (SYS_GPS ? (MAX_PRN_GPS - MIN_PRN_GPS + 1) : 0)

#define MIN_PRN_GLO     (SYS_GLO ?  1 : 0)
#define MAX_PRN_GLO     (SYS_GLO ? 27 : 0)
#define NSAT_GLO        (SYS_GLO ? (MAX_PRN_GLO - MIN_PRN_GLO + 1) : 0)

#define MIN_PRN_GAL     (SYS_GAL ?  1 : 0)
#define MAX_PRN_GAL     (SYS_GAL ? 36 : 0)
#define NSAT_GAL        (SYS_GAL ? (MAX_PRN_GAL - MIN_PRN_GAL + 1) : 0)

#define MIN_PRN_BDS     (SYS_BDS ?  1 : 0)
#define MAX_PRN_BDS     (SYS_BDS ? 63 : 0)
#define NSAT_BDS        (SYS_BDS ? (MAX_PRN_BDS - MIN_PRN_BDS + 1) : 0)

#define MIN_PRN_QZS     (SYS_QZS ? 193 : 0)
#define MAX_PRN_QZS     (SYS_QZS ? 202 : 0)
#define NSAT_QZS        (SYS_QZS ? (MAX_PRN_QZS - MIN_PRN_QZS + 1) : 0)

#define MIN_PRN_IRN     (SYS_IRN ?  1 : 0)
#define MAX_PRN_IRN     (SYS_IRN ? 14 : 0)
#define NSAT_IRN        (SYS_IRN ? (MAX_PRN_IRN - MIN_PRN_IRN + 1) : 0)

#define MIN_PRN_SBS     (SYS_SBS ? 120 : 0)
#define MAX_PRN_SBS     (SYS_SBS ? 158 : 0)
#define NSAT_SBS        (SYS_SBS ? (MAX_PRN_SBS - MIN_PRN_SBS + 1) : 0)

#define NSAT            (NSAT_GPS + NSAT_GLO + NSAT_GAL + NSAT_BDS + NSAT_QZS + NSAT_IRN + NSAT_SBS)

#define SOLQ_NONE       0                       // Solution quality: None
#define SOLQ_SPP        1                       // Solution quality: SPP
#define SOLQ_RTK        2                       // Solution quality: RTK

#define FIXQ_FLOAT      0                       // Fix quality: Float
#define FIXQ_INT        1                       // Fix quality: Integer

#define PROCMODE_SPP    0                       // Processing mode: SPP
#define PROCMODE_RTK    1                       // Processing mode: RTK

#define ENGINE_LSQ      0                       // Engine: Least Square Estimation
#define ENGINE_EKF      1                       // Engine: Extended Kalman Filter

#define EPHOPT_BRDC     0                       // Epoch option: Broadcast epheme
#define EPHOPT_PREC     1                       // Epoch option: Precise ephemeris

#define POSOPT_EST      0                       // Position option: Estimate receiver position
#define POSOPT_FIX      1                       // Position option: Fix receiver position

#define IONOOPT_OFF     0                       // Ionosphere option: Off
#define IONOOPT_BRDC    1                       // Ionosphere option: Broadcast klobuchar model
#define IONOOPT_EST     2                       // Ionosphere option: Estimate slant ionospheric delay

#define TROPOOPT_OFF    0                       // Troposphere option: Off
#define TROPOOPT_SAAS   1                       // Troposphere option: Saastamoinen model
#define TROPOOPT_EST    2                       // Troposphere option: Estimate zenith wet delay

#define DYNOPT_OFF      0                       // Dynamic option: no dynamics model
#define DYNOPT_ON       1                       // Dynamic option: Estimate receiver velocities and accelerations
#define DYNOPT_STATIC   2                       // Dynamic option: Static mode

#define MAX_DTOE_GPS    7200.0                  // Maximum ephemeris age tolerance: GPS
#define MAX_DTOE_GLO    1800.0                  // Maximum ephemeris age tolerance: GLO
#define MAX_DTOE_GAL    14400.0                 // Maximum ephemeris age tolerance: GAL
#define MAX_DTOE_BDS    21600.0                 // Maximum ephemeris age tolerance: BDS
#define MAX_DTOE_QZS    7200.0                  // Maximum ephemeris age tolerance: QZS
#define MAX_DTOE_IRN    7200.0                  // Maximum ephemeris age tolerance: IRN
#define MAX_DTOE_SBS    360.0                   // Maximum ephemeris age tolerance: SBS

#define OMGE_GPS        7.2921151467E-5         // Earth's angular velocity (GPS)
#define OMGE_GLO        7.2921150000E-5         // Earth's angular velocity (GLO)
#define OMGE_GAL        7.2921151467E-5         // Earth's angular velocity (GAL)
#define OMGE_BDS        7.2921150000E-5         // Earth's angular velocity (BDS)
#define OMGE_QZS        7.2921151467E-5         // Earth's angular velocity (QZS)
#define OMGE_IRN        7.2921151467E-5         // Earth's angular velocity (IRN)
#define OMGE_SBS        7.2921151467E-5         // Earth's angular velocity (SBS)

#define MU_GPS          3.986005000E14          // Earth's gravitational constant: GPS
#define MU_GLO          3.986004400E14          // Earth's gravitational constant: GLONASS
#define MU_GAL          3.986004418E14          // Earth's gravitational constant: Galileo
#define MU_BDS          3.986004418E14          // Earth's gravitational constant: BDS
#define MU_QZS          3.986005000E14          // Earth's gravitational constant: QZSS
#define MU_IRN          3.986005000E14          // Earth's gravitational constant: IRNSS
#define MU_SBS          3.986005000E14          // Earth's gravitational constant: SBAS

#define ERR_FACTOR_GPS  1.0                     // Error factor: GPS
#define ERR_FACTOR_GLO  1.5                     // Error factor: GLO
#define ERR_FACTOR_GAL  1.0                     // Error factor: GAL
#define ERR_FACTOR_BDS  1.0                     // Error factor: BDS
#define ERR_FACTOR_QZS  1.0                     // Error factor: QZS
#define ERR_FACTOR_IRN  1.5                     // Error factor: IRN
#define ERR_FACTOR_SBS  3.0                     // Error factor: SBS

#define STD_EPH_GLO     5.0                     // Standard deviation of GLONASS broadcast ephemeris and clock [m]
#define STD_IONO        5.0                     // Standard deviation of ionospheric delay [m]
#define STD_TROP        5.0                     // Standard deviation of tropospheric delay [m]
#define STD_TGD_CORR    0.3                     // Standard deviation of TGD correction [m]
#define STD_SAAS        0.3                     // Standard deviation of Saastamoinen model [m]
#define STD_KLOB_FACTOR 0.5                     // Factor of standard deviation of Klobuchar model

#define MAX_ITER_KEPLER 30                      // Maximum number of iterations for Kepler equation
#define MAX_ITER_LSQ    10                      // Maximum number of iterations for least square estimation

#define MAX_ERR_EPH     300.0                   // Maximum error of broadcast ephemeris
#define MAX_ERR_GAL     500.0                   // Maximum error of Galileo broadcast ephemeris

// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // CONST_H
