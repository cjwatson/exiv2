// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
// *****************************************************************************
// included header files
#include "types.hpp"
#include "minoltamn_int.hpp"
#include "sonymn_int.hpp"
#include "tags_int.hpp"
#include "tiffcomposite_int.hpp"
#include "value.hpp"
#include "exif.hpp"
#include "i18n.h"                // NLS support.
#include "utils.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    // -- Standard Sony Makernotes tags ---------------------------------------------------------------

    //! Lookup table to translate Sony Auto HDR values to readable labels
    constexpr TagDetails sonyHDRMode[] = {
        { 0x00000, N_("Off")   },
        { 0x10001, N_("Auto")  },
        { 0x10010, "1"         },
        { 0x10012, "2"         },
        { 0x10014, "3"         },
        { 0x10016, "4"         },
        { 0x10018, "5"         }
    };

    //! Lookup table to translate Sony model ID values to readable labels
    constexpr TagDetails sonyModelId[] = {
        { 2, "DSC-R1"                   },
        { 256, "DSLR-A100"              },
        { 257, "DSLR-A900"              },
        { 258, "DSLR-A700"              },
        { 259, "DSLR-A200"              },
        { 260, "DSLR-A350"              },
        { 261, "DSLR-A300"              },
        { 262, "DSLR-A900 (APS-C mode)" },
        { 263, "DSLR-A380/A390"         },
        { 264, "DSLR-A330"              },
        { 265, "DSLR-A230"              },
        { 266, "DSLR-A290"              },
        { 269, "DSLR-A850"              },
        { 270, "DSLR-A850 (APS-C mode)" },
        { 273, "DSLR-A550"              },
        { 274, "DSLR-A500"              },
        { 275, "DSLR-A450"              },
        { 278, "NEX-5"                  },
        { 279, "NEX-3"                  },
        { 280, "SLT-A33"                },
        { 281, "SLT-A55 / SLT-A55V"     },
        { 282, "DSLR-A560"              },
        { 283, "DSLR-A580"              },
        { 284, "NEX-C3"                 },
        { 285, "SLT-A35"                },
        { 286, "SLT-A65 / SLT-A65V"     },
        { 287, "SLT-A77 / SLT-A77V"     },
        { 288, "NEX-5N"                 },
        { 289, "NEX-7"                  },
        { 290, "NEX-VG20E"              },
        { 291, "SLT-A37"                },
        { 292, "SLT-A57"                },
        { 293, "NEX-F3"                 },
        { 294, "SLT-A99 / SLT-A99V"     },
        { 295, "NEX-6"                  },
        { 296, "NEX-5R"                 },
        { 297, "DSC-RX100"              },
        { 298, "DSC-RX1"                },
        { 299, "NEX-VG900"              },
        { 300, "NEX-VG30E"              },
        { 302, "ILCE-3000 / ILCE-3500"  },
        { 303, "SLT-A58"                },
        { 305, "NEX-3N"                 },
        { 306, "ILCE-7"                 },
        { 307, "NEX-5T"                 },
        { 308, "DSC-RX100M2"            },
        { 309, "DSC-RX10"               },
        { 310, "DSC-RX1R"               },
        { 311, "ILCE-7R"                },
        { 312, "ILCE-6000"              },
        { 313, "ILCE-5000"              },
        { 317, "DSC-RX100M3"            },
        { 318, "ILCE-7S"                },
        { 319, "ILCA-77M2"              },
        { 339, "ILCE-5100"              },
        { 340, "ILCE-7M2"               },
        { 341, "DSC-RX100M4"            },
        { 342, "DSC-RX10M2"             },
        { 344, "DSC-RX1RM2"             },
        { 346, "ILCE-QX1"               },
        { 347, "ILCE-7RM2"              },
        { 350, "ILCE-7SM2"              },
        { 353, "ILCA-68"                },
        { 354, "ILCA-99M2"              },
        { 355, "DSC-RX10M3"             },
        { 356, "DSC-RX100M5"            },
        { 357, "ILCE-6300"              }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    constexpr TagDetails print0xb025[] = {
        { 0,  N_("Off")           },
        { 1,  N_("Standard")      },
        { 2,  N_("Advanced Auto") },
        { 3,  N_("Auto")          },
        { 8,  N_("Advanced Lv1")  },
        { 9,  N_("Advanced Lv2")  },
        { 10, N_("Advanced Lv3")  },
        { 11, N_("Advanced Lv4")  },
        { 12, N_("Advanced Lv5")  },
        { 16, "1"                 },
        { 17, "2"                 },
        { 18, "3"                 },
        { 19, "4"                 },
        { 20, "5"                 }
    };

    //! Lookup table to translate Sony exposure mode values to readable labels
    constexpr TagDetails sonyExposureMode[] = {
        { 0,     N_("Auto")                     },
        { 1,     N_("Portrait")                 },
        { 2,     N_("Beach")                    },
        { 3,     N_("Sports")                   },
        { 4,     N_("Snow")                     },
        { 5,     N_("Landscape")                },
        { 6,     N_("Program")                  },
        { 7,     N_("Aperture priority")        },
        { 8,     N_("Shutter priority")         },
        { 9,     N_("Night Scene / Twilight")   },
        { 10,    N_("Hi-Speed Shutter")         },
        { 11,    N_("Twilight Portrait")        },
        { 12,    N_("Soft Snap / Portrait")     },
        { 13,    N_("Fireworks")                },
        { 14,    N_("Smile Shutter")            },
        { 15,    N_("Manual")                   },
        { 18,    N_("High Sensitivity")         },
        { 19,    N_("Macro")                    },
        { 20,    N_("Advanced Sports Shooting") },
        { 29,    N_("Underwater")               },
        { 33,    N_("Food")                     },
        { 34,    N_("Sweep Panorama")           },
        { 35,    N_("Handheld Night Shot")      },
        { 36,    N_("Anti Motion Blur")         },
        { 37,    N_("Pet")                      },
        { 38,    N_("Backlight Correction HDR") },
        { 39,    N_("Superior Auto")            },
        { 40,    N_("Background Defocus")       },
        { 41,    N_("Soft Skin")                },
        { 42,    N_("3D Image")                 },
        { 65535, N_("n/a")                      }
    };

    //! Lookup table to translate Sony JPEG Quality values to readable labels
    constexpr TagDetails sonyJPEGQuality[] = {
        { 0,     N_("Normal")       },
        { 1,     N_("Fine")         },
        { 2,     N_("Extra Fine")   },
        { 65535, N_("n/a")          }
    };

    //! Lookup table to translate Sony anti-blur values to readable labels
    constexpr TagDetails sonyAntiBlur[] = {
        { 0,     N_("Off")             },
        { 1,     N_("On (Continuous)") },
        { 2,     N_("On (Shooting)")   },
        { 65535, N_("n/a")             }
    };

    //! Lookup table to translate Sony dynamic range optimizer values to readable labels
    constexpr TagDetails print0xb04f[] = {
        { 0, N_("Off")      },
        { 1, N_("Standard") },
        { 2, N_("Plus")     }
    };

    //! Lookup table to translate Sony Intelligent Auto values to readable labels
    constexpr TagDetails sonyIntelligentAuto[] = {
        { 0, N_("Off")      },
        { 1, N_("On")       },
        { 2, N_("Advanced") }
    };

    //! Lookup table to translate Sony WB values to readable labels
    constexpr TagDetails sonyWhiteBalance[] = {
        { 0,  N_("Auto")                       },
        { 4,  N_("Manual")                     },
        { 5,  N_("Daylight")                   },
        { 6,  N_("Cloudy")                     },
        { 7,  N_("White Fluorescent")          },
        { 8,  N_("Cool White Fluorescent")     },
        { 9,  N_("Day White Fluorescent")      },
        { 10, N_("Incandescent2")              },
        { 11, N_("Warm White Fluorescent")     },
        { 14, N_("Incandescent")               },
        { 15, N_("Flash")                      },
        { 17, N_("Underwater 1 (Blue Water)")  },
        { 18, N_("Underwater 2 (Green Water)") }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    constexpr TagDetails sonyFocusMode[] = {
        { 1,     "AF-S"             },
        { 2,     "AF-C"             },
        { 4,     N_("Permanent-AF") },
        { 65535, N_("n/a")          }
    };

    //! Lookup table to translate Sony AF mode values to readable labels
    constexpr TagDetails sonyAFMode[] = {
        { 0,     N_("Default")          },
        { 1,     N_("Multi AF")         },
        { 2,     N_("Center AF")        },
        { 3,     N_("Spot AF")          },
        { 4,     N_("Flexible Spot AF") },
        { 6,     N_("Touch AF")         },
        { 14,    N_("Manual Focus")     },
        { 15,    N_("Face Detected")    },
        { 65535, N_("n/a")              }
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    constexpr TagDetails sonyAFIlluminator[] = {
        { 0,     N_("Off") },
        { 1,     N_("Auto")  },
        { 65535, N_("n/a") }
    };

    //! Lookup table to translate Sony macro mode values to readable labels
    constexpr TagDetails sonyMacroMode[] = {
        { 0,     N_("Off")         },
        { 1,     N_("On")          },
        { 2,     N_("Close Focus") },
        { 65535, N_("n/a")         }
    };

    //! Lookup table to translate Sony flash level values to readable labels
    constexpr TagDetails sonyFlashLevel[] = {
        { -32768, N_("Low")    },
        { -1,     N_("n/a")    },
        { 0,      N_("Normal") },
        { 32767,  N_("High")   }
    };

    //! Lookup table to translate Sony release mode values to readable labels
    constexpr TagDetails sonyReleaseMode[] = {
        { 0,     N_("Normal")                   },
        { 2,     N_("Burst")                    },
        { 5,     N_("Exposure Bracketing")      },
        { 6,     N_("White Balance Bracketing") },
        { 65535, N_("n/a")                      }
    };

    //! Lookup table to translate Sony Release Mode 2 values to readable labels
    constexpr TagDetails sonyReleaseMode2[] = {
        { 0,     N_("Normal")                                             },
        { 1,     N_("Continuous")                                         },
        { 2,     N_("Continuous - Exposure Bracketing")                   },
        { 3,     N_("DRO or White Balance Bracketing")                    },
        { 5,     N_("Continuous - Burst")                                 },
        { 6,     N_("Single Frame - Capture During Movie")                },
        { 7,     N_("Continuous - Sweep Panorama")                        },
        { 8,     N_("Continuous - Anti-Motion Blur, Hand-held Twilight")  },
        { 9,     N_("Continuous - HDR")                                   },
        { 10,    N_("Continuous - Background defocus")                    },
        { 13,    N_("Continuous - 3D Sweep Panorama")                     },
        { 15,    N_("Continuous - High Resolution Sweep Panorama")        },
        { 16,    N_("Continuous - 3D Image")                              },
        { 17,    N_("Continuous - Burst 2")                               },
        { 18,    N_("Normal - iAuto+")                                    },
        { 19,    N_("Continuous - Speed/Advance Priority")                },
        { 20,    N_("Continuous - Multi-Frame NR")                        },
        { 23,    N_("Single-frame - Exposure Bracketing")                 },
        { 26,    N_("Continuous Low")                                     },
        { 27,    N_("Continuous - High Sensitivity")                      },
        { 28,    N_("Smile Shutter")                                      },
        { 29,    N_("Continuous - Tele-zoom Advance Priority")            },
        { 146,   N_("Single Frame - Movie Capture")                       }
    };

    //! Lookup table to translate Sony sequence number values to readable labels
    constexpr TagDetails sonySequenceNumber[] = {
        { 0,     N_("Single")                    },
        { 65535, N_("n/a")                       }
    };

    //! Lookup table to translate Sony long exposure noise reduction values to readable labels
    constexpr TagDetails sonyLongExposureNoiseReduction[] = {
        { 0,     N_("Off") },
        { 1,     N_("On")  },
        { 65535, N_("n/a") }
    };

    std::ostream& SonyMakerNote::print0xb000(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 4)
        {
            os << "(" << value << ")";
        }
        else
        {
            std::string val = value.toString(0) + value.toString(1) + value.toString(2) + value.toString(3);
            if      (val == "0002") os << "JPEG";
            else if (val == "1000") os << "SR2";
            else if (val == "2000") os << "ARW 1.0";
            else if (val == "3000") os << "ARW 2.0";
            else if (val == "3100") os << "ARW 2.1";
            else if (val == "3200") os << "ARW 2.2";
            else if (val == "3300") os << "ARW 2.3";
            else if (val == "3310") os << "ARW 2.3.1";
            else if (val == "3320") os << "ARW 2.3.2";
            else                    os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& SonyMakerNote::printImageSize(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() == 2)
            os << value.toString(0) << " x " << value.toString(1);
        else
            os << "(" << value << ")";

        return os;
    }

    // Sony MakerNote Tag Info
    constexpr TagInfo SonyMakerNote::tagInfo_[] = {
        {0x0102, "Quality", N_("Image Quality"),
                N_("Image quality"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyImageQuality},
        {0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"),
                N_("Flash exposure compensation in EV"),
                sony1Id, makerTags, signedRational, -1, print0x9204},
        {0x0105, "Teleconverter", N_("Teleconverter Model"),
                N_("Teleconverter Model"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel},
        {0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune Value"),
                sony1Id, makerTags, unsignedLong, -1, printValue},
        {0x0114, "CameraSettings", N_("Camera Settings"),
                N_("Camera Settings"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x0115, "WhiteBalance", N_("White Balance"),
                N_("White balance"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyWhiteBalanceStd},
        {0x0116, "0x0116", "0x0116",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x0E00, "PrintIM", N_("Print IM"),
                N_("PrintIM information"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x1000, "MultiBurstMode", N_("Multi Burst Mode"),
                N_("Multi Burst Mode"),
                sony1Id, makerTags, undefined, -1, printMinoltaSonyBoolValue},
        {0x1001, "MultiBurstImageWidth", N_("Multi Burst Image Width"),
                N_("Multi Burst Image Width"),
                sony1Id, makerTags, unsignedShort, -1, printValue},
        {0x1002, "MultiBurstImageHeight", N_("Multi Burst Image Height"),
                N_("Multi Burst Image Height"),
                sony1Id, makerTags, unsignedShort, -1, printValue},
        // TODO : Implement Panorama tags decoding.
        {0x1003, "Panorama", N_("Panorama"),
                N_("Panorama"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x2000, "0x2000", "0x2000",
                N_("Unknown"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x2001, "PreviewImage", N_("Preview Image"),
                N_("JPEG preview image"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0x2002, "0x2002", "0x2002",
                N_("Unknown"),
                sony1Id, makerTags, unsignedLong, -1, printValue},
        {0x2003, "0x2003", "0x2003",
                N_("Unknown"),
                sony1Id, makerTags, asciiString, -1, printValue},
        {0x2004, "Contrast", "Contrast",
                N_("Contrast"),
                sony1Id, makerTags, signedLong, -1, printValue},
        {0x2005, "Saturation", "Saturation",
                N_("Saturation"),
                sony1Id, makerTags, signedLong, -1, printValue},
        {0x2006, "0x2006", "0x2006",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue},
        {0x2007, "0x2007", "0x2007",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue},
        {0x2008, "0x2008", "0x2008",
                N_("Unknown"),
                sony1Id, makerTags, signedLong, -1, printValue},
        {0x2009, "0x2009", "0x2009",
                N_("Unknown"),
                sony1Id, makerTags, unsignedShort, -1, printValue},
        {0x200A, "AutoHDR", N_("Auto HDR"),
                N_("High Definition Range Mode"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyHDRMode)},
        // TODO : Implement Shot Info tags decoding.
        {0x3000, "ShotInfo", N_("Shot Info"),
                N_("Shot Information"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0xB000, "FileFormat", N_("File Format"),
                N_("File Format"),
                sony1Id, makerTags, unsignedByte, -1, print0xb000},
        {0xB001, "SonyModelID", N_("Sony Model ID"),
                N_("Sony Model ID"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyModelId)},
        {0xB020, "ColorReproduction", N_("Color Reproduction"),
                N_("Color Reproduction"),
                sony1Id, makerTags, asciiString, -1, printValue},
        {0xb021, "ColorTemperature", N_("Color Temperature"),
                N_("Color Temperature"),
                sony1Id, makerTags, unsignedLong, -1, printValue},
        {0xB022, "ColorCompensationFilter", N_("Color Compensation Filter"),
                N_("Color Compensation Filter: negative is green, positive is magenta"),
                sony1Id, makerTags, unsignedLong, -1, printValue},
        {0xB023, "SceneMode", N_("Scene Mode"),
                N_("Scene Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonySceneMode},
        {0xB024, "ZoneMatching", N_("Zone Matching"),
                N_("Zone Matching"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyZoneMatching},
        {0xB025, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedLong, -1, EXV_PRINT_TAG(print0xb025)},
        {0xB026, "ImageStabilization", N_("Image Stabilization"),
                N_("Image stabilization"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyBoolValue},
        {0xB027, "LensID", N_("Lens ID"),
                N_("Lens identifier"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyLensID},
        {0xB028, "MinoltaMakerNote", N_("Minolta MakerNote"),
                N_("Minolta MakerNote"),
                sony1Id, makerTags, undefined, -1, printValue},
        {0xB029, "ColorMode", N_("Color Mode"),
                N_("Color Mode"),
                sony1Id, makerTags, unsignedLong, -1, printMinoltaSonyColorMode},
        {0xB02B, "FullImageSize", N_("Full Image Size"),
                N_("Full Image Size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize},
        {0xB02C, "PreviewImageSize", N_("Preview Image Size"),
                N_("Preview image size"),
                sony1Id, makerTags, unsignedLong, -1, printImageSize},
        {0xB040, "Macro", N_("Macro"),
                N_("Macro"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyMacroMode)},
        {0xB041, "ExposureMode", N_("Exposure Mode"),
                N_("Exposure Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyExposureMode)},
        {0xB042, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyFocusMode)},
        {0xB043, "AFMode", N_("AF Mode"),
                N_("AF Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFMode)},
        {0xB044, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFIlluminator)},
        {0xB047, "JPEGQuality", N_("JPEG Quality"),
                N_("JPEG Quality"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyJPEGQuality)},
        {0xB048, "FlashLevel", N_("Flash Level"),
                N_("Flash Level"),
                sony1Id, makerTags, signedShort, -1, EXV_PRINT_TAG(sonyFlashLevel)},
        {0xB049, "ReleaseMode", N_("Release Mode"),
                N_("Release Mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyReleaseMode)},
        {0xB04A, "SequenceNumber", N_("Sequence Number"),
                N_("Shot number in continuous burst mode"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonySequenceNumber)},
        {0xB04B, "AntiBlur", N_("Anti-Blur"),
                N_("Anti-Blur"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAntiBlur)},
        {0xB04E, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyLongExposureNoiseReduction)},
        {0xB04F, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"),
                N_("Dynamic Range Optimizer"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(print0xb04f)},
        {0xB052, "IntelligentAuto", N_("Intelligent Auto"),
                N_("Intelligent Auto"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyIntelligentAuto)},
        {0xB054, "WhiteBalance2", N_("White Balance 2"),
                N_("White balance 2"),
                sony1Id, makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyWhiteBalance)},
        // End of list marker
        {0xffff, "(UnknownSony1MakerNoteTag)", "(UnknownSony1MakerNoteTag)",
                N_("Unknown Sony1MakerNote tag"),
                sony1Id, makerTags, asciiString, -1, printValue},
    };

    const TagInfo* SonyMakerNote::tagList()
    {
        return tagInfo_;
    }

    // -- Sony camera settings ---------------------------------------------------------------

    //! Lookup table to translate Sony camera settings drive mode values to readable labels
    constexpr TagDetails sonyDriveModeStd[] = {
        { 0x01, N_("Single Frame")                      },
        { 0x02, N_("Continuous High")                   },
        { 0x04, N_("Self-timer 10 sec")                 },
        { 0x05, N_("Self-timer 2 sec, Mirror Lock-up")  },
        { 0x06, N_("Single-frame Bracketing")           },
        { 0x07, N_("Continuous Bracketing")             },
        { 0x0a, N_("Remote Commander")                  },
        { 0x0b, N_("Mirror Lock-up")                    },
        { 0x12, N_("Continuous Low")                    },
        { 0x18, N_("White Balance Bracketing Low")      },
        { 0x19, N_("D-Range Optimizer Bracketing Low")  },
        { 0x28, N_("White Balance Bracketing High")     },
        { 0x29, N_("D-Range Optimizer Bracketing High") },
        { 0x29, N_("D-Range Optimizer Bracketing High") } // To silence compiler warning
    };

    //! Lookup table to translate Sony camera settings focus mode values to readable labels
    constexpr TagDetails sonyCSFocusMode[] = {
        { 0, N_("Manual") },
        { 1, "AF-S"       },
        { 2, "AF-C"       },
        { 3, "AF-A"       }
    };

    //! Lookup table to translate Sony camera settings metering mode values to readable labels
    constexpr TagDetails sonyMeteringMode[] = {
        { 1, N_("Multi-segment")           },
        { 2, N_("Center weighted average") },
        { 4, N_("Spot")                    }
    };

    //! Lookup table to translate Sony camera settings creative style values to readable labels
    constexpr TagDetails sonyCreativeStyle[] = {
        { 1,    N_("Standard")            },
        { 2,    N_("Vivid")               },
        { 3,    N_("Portrait")            },
        { 4,    N_("Landscape")           },
        { 5,    N_("Sunset")              },
        { 6,    N_("Night View/Portrait") },
        { 8,    N_("Black & White")       },
        { 9,    N_("Adobe RGB")           },
        { 11,   N_("Neutral")             },
        { 12,   N_("Clear")               },
        { 13,   N_("Deep")                },
        { 14,   N_("Light")               },
        { 15,   N_("Autumn")              },
        { 16,   N_("Sepia")               }
    };

    //! Lookup table to translate Sony camera settings flash mode values to readable labels
    constexpr TagDetails sonyFlashMode[] = {
        { 0, N_("ADI") },
        { 1, N_("TTL") },
    };

    //! Lookup table to translate Sony AF illuminator values to readable labels
    constexpr TagDetails sonyAFIlluminatorCS[] = {
        { 0, N_("Auto") },
        { 1, N_("Off")  }
    };

    //! Lookup table to translate Sony camera settings image style values to readable labels
    constexpr TagDetails sonyImageStyle[] = {
        { 1,    N_("Standard")            },
        { 2,    N_("Vivid")               },
        { 3,    N_("Portrait")            },
        { 4,    N_("Landscape")           },
        { 5,    N_("Sunset")              },
        { 7,    N_("Night View/Portrait") },
        { 8,    N_("B&W")                 },
        { 9,    N_("Adobe RGB")           },
        { 11,   N_("Neutral")             },
        { 129,  N_("StyleBox1")           },
        { 130,  N_("StyleBox2")           },
        { 131,  N_("StyleBox3")           },
        { 132,  N_("StyleBox4")           },
        { 133,  N_("StyleBox5")           },
        { 134,  N_("StyleBox6")           }
    };

    //! Lookup table to translate Sony camera settings exposure program values to readable labels
    constexpr TagDetails sonyExposureProgram[] = {
        { 0,    N_("Auto")                      },
        { 1,    N_("Manual")                    },
        { 2,    N_("Program AE")                },
        { 3,    N_("Aperture-priority AE")      },
        { 4,    N_("Shutter speed priority AE") },
        { 8,    N_("Program Shift A")           },
        { 9,    N_("Program Shift S")           },
        { 16,   N_("Portrait")                  },
        { 17,   N_("Sports")                    },
        { 18,   N_("Sunset")                    },
        { 19,   N_("Night Portrait")            },
        { 20,   N_("Landscape")                 },
        { 21,   N_("Macro")                     },
        { 35,   N_("Auto No Flash")             }
    };

    //! Lookup table to translate Sony camera settings image size values to readable labels
    constexpr TagDetails sonyImageSize[] = {
        { 1, N_("Large")  },
        { 2, N_("Medium") },
        { 3, N_("Small")  }
    };

    //! Lookup table to translate Sony aspect ratio values to readable labels
    constexpr TagDetails sonyAspectRatio[] = {
        { 1, "3:2"   },
        { 2, "16:9"  }
    };

    //! Lookup table to translate Sony exposure level increments values to readable labels
    constexpr TagDetails sonyExposureLevelIncrements[] = {
        { 33, "1/3 EV" },
        { 50, "1/2 EV" }
    };

    // Sony Camera Settings Tag Info
    // NOTE: all are for A200, A230, A300, A350, A700, A850 and A900 Sony model excepted
    // some entries which are only relevant with A700.

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    constexpr TagInfo SonyMakerNote::tagInfoCs_[] = {
        // NOTE: A700 only
        {0x0004, "DriveMode", N_("Drive Mode"),
                N_("Drive Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyDriveModeStd)},
        // NOTE: A700 only
        {0x0006, "WhiteBalanceFineTune", N_("White Balance Fine Tune"),
                N_("White Balance Fine Tune"),
                sony1CsId, makerTags, signedShort, 1, printValue},
        {0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)},
        {0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode},
        {0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint},
        {0x0015, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)},
        {0x0016, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x0018, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode},
        {0x0019, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x001A, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)},
        {0x001C, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x001D, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x001E, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x001F, "ZoneMatchingValue", N_("Zone Matching Value"),
                N_("Zone Matching Value"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x0022, "Brightness", N_("Brightness"),
                N_("Brightness"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        {0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)},
        // NOTE: A700 only
        {0x0028, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"),
                N_("Priority Setup Shutter Release"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease},
        // NOTE: A700 only
        {0x0029, "AFIlluminator", N_("AF Illuminator"),
                N_("AF Illuminator"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAFIlluminatorCS)},
        // NOTE: A700 only
        {0x002A, "AFWithShutter", N_("AF With Shutter"),
                N_("AF With Shutter"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolInverseValue},
        // NOTE: A700 only
        {0x002B, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"),
                N_("Long Exposure Noise Reduction"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
        // NOTE: A700 only
        {0x002C, "HighISONoiseReduction", N_("High ISO NoiseReduction"),
                N_("High ISO NoiseReduction"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
        // NOTE: A700 only
        {0x002D, "ImageStyle", N_("Image Style"),
                N_("Image Style"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageStyle)},
        {0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)},
        {0x003D, "ImageStabilization", N_("Image Stabilization"),
                N_("Image Stabilization"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
        {0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyRotation},
        {0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)},
        {0x0055, "AspectRatio", N_("Aspect Ratio"),
                N_("Aspect Ratio"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyAspectRatio)},
        {0x0056, "Quality", N_("Quality"),
                N_("Quality"),
                sony1CsId, makerTags, unsignedShort, 1, printMinoltaSonyQualityCs},
        {0x0058, "ExposureLevelIncrements", N_("Exposure Level Increments"),
                N_("Exposure Level Increments"),
                sony1CsId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureLevelIncrements)},
        // End of list marker
        {0xffff, "(UnknownSony1CsTag)", "(UnknownSony1CsTag)",
                N_("Unknown Sony1 Camera Settings tag"),
                sony1CsId, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* SonyMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    // -- Sony camera settings 2 ---------------------------------------------------------------

    // Sony Camera Settings Tag Version 2 Info
    // NOTE: for A330, A380, A450, A500, A550 Sony model

    // Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

    constexpr TagInfo SonyMakerNote::tagInfoCs2_[] = {
        {0x0010, "FocusMode", N_("Focus Mode"),
                N_("Focus Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCSFocusMode)},
        {0x0011, "AFAreaMode", N_("AF Area Mode"),
                N_("AF Area Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyAFAreaMode},
        {0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"),
                N_("Local AF Area Point"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint},
        {0x0013, "MeteringMode", N_("Metering Mode"),
                N_("Metering Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)},
        {0x0014, "ISOSetting", N_("ISO Setting"),
                N_("ISO Setting"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
        {0x0016, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"),
                N_("Dynamic Range Optimizer Mode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode},
        {0x0017, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"),
                N_("Dynamic Range Optimizer Level"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
        {0x0018, "CreativeStyle", N_("Creative Style"),
                N_("Creative Style"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)},
        {0x0019, "Sharpness", N_("Sharpness"),
                N_("Sharpness"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
        {0x001A, "Contrast", N_("Contrast"),
                N_("Contrast"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
        {0x001B, "Saturation", N_("Saturation"),
                N_("Saturation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
        {0x0023, "FlashMode", N_("FlashMode"),
                N_("FlashMode"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyFlashMode)},
        {0x003C, "ExposureProgram", N_("Exposure Program"),
                N_("Exposure Program"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)},
        {0x003F, "Rotation", N_("Rotation"),
                N_("Rotation"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printMinoltaSonyRotation},
        {0x0054, "SonyImageSize", N_("Sony Image Size"),
                N_("Sony Image Size"),
                sony1Cs2Id, makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)},
        // End of list marker
        {0xffff, "(UnknownSony1Cs2Tag)", "(UnknownSony1Cs2Tag)",
                N_("Unknown Sony1 Camera Settings 2 tag"),
                sony1Cs2Id, makerTags, unsignedShort, 1, printValue},
    };

    const TagInfo* SonyMakerNote::tagListCs2()
    {
        return tagInfoCs2_;
    }

    //! Lookup table to translate Sony2Fp AF Area Mode values to readable labels
    constexpr TagDetails sony2FpAFAreaMode[] = {
        { 0,   N_("Multi")                               },
        { 1,   N_("Center")                              },
        { 2,   N_("Spot")                                },
        { 3,   N_("Flexible Spot")                       },
        { 10,  N_("Selective (for Miniature effect)")    },
        { 11,  N_("Zone")                                },
        { 12,  N_("Expanded Flexible Spot")              },
        { 14,  N_("Tracking")                            },
        { 15,  N_("Face Tracking")                       },
        { 20,  N_("Animal Eye Tracking")                 },
        { 255, N_("Manual")                              }
    };

    //! Sony Tag 9402 Sony2Fp (FocusPosition)
    constexpr TagInfo SonyMakerNote::tagInfoFp_[] = {
        {  0x04, "AmbientTemperature", N_("Ambient temperature"), N_("Temperature of the surroundings (in degrees Celsius)"), sony2FpId, makerTags, signedByte, 1, printTemperatureInDegC},
        {  0x16, "FocusMode"         , N_("Focus mode")         , N_("Focus mode")          , sony2FpId, makerTags, unsignedByte, 1, printSony2FpFocusMode},
        {  0x17, "AFAreaMode"        , N_("AF area mode")       , N_("Auto focus area mode"), sony2FpId, makerTags, unsignedByte, 1, EXV_PRINT_TAG(sony2FpAFAreaMode)},
        {  0x2d, "FocusPosition2"    , N_("Focus position 2")   , N_("Focus position 2")    , sony2FpId, makerTags, unsignedByte, 1, printSony2FpFocusPosition2},
        // End of list marker
        {0xffff, "(UnknownSony2FpTag)", "(Unknown Sony2Fp tag)"   , "(Unknown Sony2Fp tag)"     , sony2FpId, makerTags, unsignedByte, 1, printValue},
    };

    const TagInfo* SonyMakerNote::tagListFp()
    {
        return tagInfoFp_;
    }

    std::ostream& SonyMakerNote::printSony2FpFocusMode(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1)
            os << value;
        else {
            long val = (value.toLong() & 0x7F);
            switch (val) {
            case 0:
                os << N_("Manual");
                break;
            case 2:
                os << N_("AF-S");
                break;
            case 3:
                os << N_("AF-C");
                break;
            case 4:
                os << N_("AF-A");
                break;
            case 6:
                os << N_("DMF");
                break;
            default:
                os << "(" << val << ")";
            }
        }

        return os;
    }

    std::ostream& SonyMakerNote::printSony2FpFocusPosition2(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            os << "(" << value << ")";
        else {
            auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
            if (pos == metadata->end())
                return os << "(" << value << ")";

            // Ranges of models that do not support this tag
            std::string model = pos->toString();
            for (auto& m : { "DSC-", "Stellar" }) {
                if (Util::startsWith(model, m)) {
                    os << N_("n/a");
                    return os;
                }
            }
            long val = value.toLong();
            switch (val) {
            case 255:
                os << N_("Infinity");
                break;
            default:
                os << val;
            }
        }
        return os;
    }

    //! Sony Tag 9403 SonyMisc1
    constexpr TagInfo SonyMakerNote::tagInfoSonyMisc1_[] = {
        {0x05,  "CameraTemperature", N_("Camera temperature"),
        		N_("Internal camera temperature (in degrees Celsius)"),
				sonyMisc1Id, makerTags, signedByte, -1, printTemperatureInDegC},
        // End of list marker
        {0xffff, "(UnknownSonyMisc1Tag)", "(UnknownSonyMisc1Tag)",
        		 "(UnknownSonyMisc1Tag)",
				 sonyMisc1Id, makerTags, unsignedByte, -1, printValue}
    };

    const TagInfo* SonyMakerNote::tagListSonyMisc1()
    {
        return tagInfoSonyMisc1_;
    }

    std::ostream& SonyMakerNote::printTemperatureInDegC(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() != 1)
            os << "(" << value << ")";
        else
            os << value << " °C";

        return os;
    }

    //! Lookup table to translate Sony Exposure Program 3 values to readable labels
    constexpr TagDetails sonyExposureProgram3[] = {
        { 0,         N_("Program AE")                 },
        { 1,         N_("Aperture-priority AE")       },
        { 2,         N_("Shutter speed priority AE")  },
        { 3,         N_("Manual")                     },
        { 4,         N_("Auto")                       },
        { 5,         N_("iAuto")                      },
        { 6,         N_("Superior Auto")              },
        { 7,         N_("iAuto+")                     },
        { 8,         N_("Portrait")                   },
        { 9,         N_("Landscape")                  },
        { 10,        N_("Twilight")                   },
        { 11,        N_("Twilight Portrait")          },
        { 12,        N_("Sunset")                     },
        { 14,        N_("Action (High speed)")        },
        { 16,        N_("Sports")                     },
        { 17,        N_("Handheld Night Shot")        },
        { 18,        N_("Anti Motion Blur")           },
        { 19,        N_("High Sensitivity")           },
        { 21,        N_("Beach")                      },
        { 22,        N_("Snow")                       },
        { 23,        N_("Fireworks")                  },
        { 26,        N_("Underwater")                 },
        { 27,        N_("Gourmet")                    },
        { 28,        N_("Pet")                        },
        { 29,        N_("Macro")                      },
        { 30,        N_("Backlight Correction HDR")   },
        { 33,        N_("Sweep Panorama")             },
        { 36,        N_("Background Defocus")         },
        { 37,        N_("Soft Skin")                  },
        { 42,        N_("3D Image")                   },
        { 43,        N_("Cont. Priority AE")          },
        { 45,        N_("Document")                   },
        { 46,        N_("Party")                      }
    };

    //! Sony Tag 9404b SonyMisc2b tags
    constexpr TagInfo SonyMakerNote::tagInfoSonyMisc2b_[] = {
        {12,    "ExposureProgram", N_("Exposure program"),
                N_("Exposure program"),
                sonyMisc2bId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyExposureProgram3)},
        {14,    "IntelligentAuto", N_("Intelligent auto"),
                N_("Whether intelligent auto was used"),
                sonyMisc2bId, makerTags, unsignedByte, -1, printMinoltaSonyBoolValue},
        {30,    "LensZoomPosition", N_("Lens zoom position"),
                N_("Lens zoom position (in %)"),
                sonyMisc2bId, makerTags, unsignedShort, -1, printSonyMisc2bLensZoomPosition},
        {32,    "FocusPosition2", N_("Focus position 2"),
                N_("Focus position 2"),
                sonyMisc2bId, makerTags, unsignedByte, -1, printSonyMisc2bFocusPosition2},
        // End of list marker
        {0xffff, "(UnknownSonyMisc2bTag)", "(Unknown SonyMisc2b tag)",
                 "(Unknown SonyMisc2b tag)",
                 sonyMisc2bId, makerTags, unsignedByte, -1, printValue}
    };

    const TagInfo* SonyMakerNote::tagListSonyMisc2b()
    {
        return tagInfoSonyMisc2b_;
    }

    std::ostream& SonyMakerNote::printSonyMisc2bLensZoomPosition(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        // Models that do not support this tag
        std::string model = pos->toString();
        for (auto& m : { "SLT-", "HV", "ILCA-"}) {
            if (model.find(m) != std::string::npos)
                return os << N_("n/a");
        }

        os << std::round((value.toLong()/10.24)) << "%";

        return os;
    }

    std::ostream& SonyMakerNote::printSonyMisc2bFocusPosition2(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        // Models that do not support this tag
        std::string model = pos->toString();
        for (auto& m : { "SLT-", "HV", "ILCA-"}) {
            if (model.find(m) != std::string::npos)
                return os << N_("n/a");
        }

        return os << value;
    }

    //! Lookup table to translate Sony camera SonyMisc3c sequence length 1 values to readable labels
    constexpr TagDetails sonyMisc3cSequenceLength1[] = {
        { 0,   N_("Continuous")                     },
        { 1,   N_("1 shot")                         },
        { 2,   N_("2 shots")                        },
        { 3,   N_("3 shots")                        },
        { 4,   N_("4 shots")                        },
        { 5,   N_("5 shots")                        },
        { 6,   N_("6 shots")                        },
        { 7,   N_("7 shots")                        },
        { 9,   N_("9 shots")                        },
        { 10,  N_("10 shots")                       },
        { 12,  N_("12 shots")                       },
        { 16,  N_("16 shots")                       },
        { 100, N_("Continuous - iSweep Panorama")   },
        { 200, N_("Continuous - Sweep Panorama")    }
    };

    //! Lookup table to translate Sony camera SonyMisc3c sequence length 2 values to readable labels
    constexpr TagDetails sonyMisc3cSequenceLength2[] = {
        { 0,   N_("Continuous")                     },
        { 1,   N_("1 file")                         },
        { 2,   N_("2 files")                        },
        { 3,   N_("3 files")                        },
        { 5,   N_("5 files")                        },
        { 7,   N_("7 files")                        },
        { 9,   N_("9 files")                        },
        { 10,  N_("10 files")                       }
    };

    //! Lookup table to translate Sony camera SonyMisc3c, camera orientation values to readable labels
    constexpr TagDetails sonyMisc3cCameraOrientation[] = {
        { 1,   N_("Horizontal (normal)")        },
        { 3,   N_("Rotate 180°")               },
        { 6,   N_("Rotate 90° CW")             },
        { 8,   N_("Rotate 270° CW")            }
    };

    //! SonyMisc3c tags (Tag 9400c)
    constexpr TagInfo SonyMakerNote::tagInfoSonyMisc3c_[] = {
        {9,    "ReleaseMode2", N_("Release mode 2"),
                N_("Release mode 2"),
                sonyMisc3cId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyReleaseMode2)},
        {10,   "ShotNumberSincePowerUp",N_("Shot number since power up"),
                N_("Number of photos taken since the camera was powered up"),
                sonyMisc3cId, makerTags, unsignedLong, -1, printSonyMisc3cShotNumberSincePowerUp},
        {18,   "SequenceImageNumber", N_("Sequence image number"),
                N_("Number of images captured in burst sequence"),
                sonyMisc3cId, makerTags, unsignedLong, -1, printSonyMisc3cSequenceNumber},
        // In Exiftool, "SequenceLength1" is called "SequenceLength. Renamed due to clash of names."
        {22,   "SequenceLength1", N_("Sequence length 1"),
                N_("Length of the sequence of photos taken"),
                sonyMisc3cId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cSequenceLength1)},
        {26,   "SequenceFileNumber", N_("Sequence file number"),
                N_("File number in burst sequence"),
                sonyMisc3cId, makerTags, unsignedLong, -1, printSonyMisc3cSequenceNumber},
        // In Exiftool, "SequenceLength2" is called "SequenceLength". Renamed due to clash of names."
        {30,   "SequenceLength2", N_("Sequence length 2"),
                N_("Length of the sequence of photos taken"),
                sonyMisc3cId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cSequenceLength2)},
        {41,   "CameraOrientation", N_("Camera orientation"),
                N_("Orientation of the camera when the photo was taken"),
                sonyMisc3cId, makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cCameraOrientation)},
        {42,   "Quality2", N_("Quality 2"),
                N_("Quality 2"),
                sonyMisc3cId, makerTags, unsignedByte, -1, printSonyMisc3cQuality2},
        {71,   "SonyImageHeight", N_("Sony image height"),
                N_("Height of the image"),
                sonyMisc3cId, makerTags, unsignedShort, -1, printSonyMisc3cSonyImageHeight},
        {83,   "ModelReleaseYear", N_("Model release year"),
                N_("Year that the model of camera was released"),
                sonyMisc3cId, makerTags, unsignedByte, -1, printSonyMisc3cModelReleaseYear},
        // End of list marker
        {0xffff, "(UnknownSonyMisc3c)", "(Unknown SonyMisc3c Tag)",
                N_("Unknown SonyMisc23 tag"),
                sonyMisc3cId, makerTags, asciiString, -1, printValue},
    };

    const TagInfo* SonyMakerNote::tagListSonyMisc3c()
    {
        return tagInfoSonyMisc3c_;
    }

    std::ostream& SonyMakerNote::printSonyMisc3cShotNumberSincePowerUp(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        // Models that support this tag
        static constexpr const char* models[] = { "ILCA-68",     "ILCA-77M2",   "ILCA-99M2",
                                                  "ILCE-5000",   "ILCE-5100",   "ILCE-6000",   "ILCE-6300",   "ILCE-6500",   "ILCE-7",
                                                  "ILCE-7M2",    "ILCE-7R",     "ILCE-7RM2",   "ILCE-7S",     "ILCE-7SM2",   "ILCE-QX1",
                                                  "DSC-HX350",   "DSC-HX400V",  "DSC-HX60V",   "DSC-HX80",    "DSC-HX90",    "DSC-HX90V",
                                                  "DSC-QX30",    "DSC-RX0",     "DSC-RX1RM2",  "DSC-RX10",    "DSC-RX10M2",  "DSC-RX10M3",
                                                  "DSC-RX100M3", "DSC-RX100M4", "DSC-RX100M5", "DSC-WX220",   "DSC-WX350",   "DSC-WX500"
        };

        std::string model = pos->toString();
        for (auto& m : models) {
            if (m == model)
                return os << value.toLong();
        }

        return os << N_("n/a");
    }

    std::ostream& SonyMakerNote::printSonyMisc3cSequenceNumber(std::ostream& os, const Value& value, const ExifData*)
    {
        return (value.count() != 1) ? os << "(" << value << ")" : os << (value.toLong()+1);
    }

    std::ostream& SonyMakerNote::printSonyMisc3cQuality2(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        long val = value.toLong();
        std::string model = pos->toString();

        // Value is interpreted differently if model is in list or not
        for (auto& m : { "ILCE-1", "ILCE-7SM3", "ILME-FX3" }) {
            if (m == model) {
                switch (val) {
                case 1:
                    return os << N_("JPEG");
                case 2:
                    return os << N_("Raw");
                case 3:
                    return os << N_("Raw + JPEG");
                case 4:
                    return os << N_("HEIF");
                case 6:
                    return os << N_("Raw + HEIF");
                default:
                    return os << "(" << val << ")";
                }
            }
        }

        switch (val) {
        case 0:
            return os << N_("JPEG");
        case 1:
            return os << N_("Raw");
        case 2:
            return os << N_("Raw + JPEG");
        case 3:
            return os << N_("Raw + MPO");
        default:
            os << "(" << val << ")";
        }
        return os;
    }

    std::ostream& SonyMakerNote::printSonyMisc3cSonyImageHeight(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        std::string model = pos->toString();

        // Models that do not support this tag
        for (auto& m : { "ILCE-1", "ILCE-7SM3", "ILME-FX3" }) {
            if (m == model)
                return os << N_("n/a");
        }

        long val = value.toLong();
        return val > 0 ? os << (8*val) : os << N_("n/a");
    }

    std::ostream& SonyMakerNote::printSonyMisc3cModelReleaseYear(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        if (value.count() != 1)
            return os << "(" << value << ")";

        auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end())
            return os << "(" << value << ")";

        std::string model = pos->toString();

        // Models that do not support this tag
        for (auto& m : { "ILCE-1", "ILCE-7SM3", "ILME-FX3" }) {
            if (m == model)
                return os << N_("n/a");
        }

        long val = value.toLong();
        if (val > 99)
            return os << "(" << val << ")";

        if (val == 0)
            return os << "2000";

        return os << "20" << val;
    }

    //! Sony SInfo1 tags (ShotInfo - Tag 3000)
    constexpr TagInfo SonyMakerNote::tagInfoSonySInfo1_[] = {
        // TODO: As "FaceInfo1" (72) and "FaceInfo2" (94) are not added, including
        //       "FaceInfoOffset" (2) and "FaceInfoLength" (50) does not make sense.
        //       The values are all connected and changing one without the rest will
        //       corrupt the data.
        {6,     "SonyDateTime", N_("Sony date/time"),
                N_("Date and time when the photo was captured"),
                sonySInfo1Id, makerTags, asciiString, -1, printValue},
        {26,    "SonyImageHeight", N_("Sony image height"),
                N_("Height of the image"),
                sonySInfo1Id, makerTags, unsignedShort, -1, printValue},
        {28,    "SonyImageWidth", N_("Sony image width"),
                N_("Width of the image"),
                sonySInfo1Id, makerTags, unsignedShort, -1, printValue},
        {48,    "FacesDetected", N_("Faces detected"),
                N_("Number of faces detected in the image"),
                sonySInfo1Id, makerTags, unsignedShort, -1, printValue},
        {52,    "MetaVersion", N_("Meta version"),
                N_("Sony meta version"),
                sonySInfo1Id, makerTags, asciiString, -1, printValue},
        // TODO: Add FaceInfo1 (72) and FaceInfo2 (94) which are sub-groups of tags.
        // End of list marker
        {0xffff, "(UnknownsonySInfo1Tag)", "(Unknown SonySInfo1 Tag)",
                 "(Unknown SonySInfo1 Tag)",
                 sonySInfo1Id, makerTags, unsignedByte, -1, printValue}
    };

    const TagInfo* SonyMakerNote::tagListSonySInfo1()
    {
        return tagInfoSonySInfo1_;
    }

    //! Sony Tag 2010 Sony2010 (Miscellaneous)
    constexpr TagInfo SonyMakerNote::tagInfo2010e_[] = {
        {0, "SequenceImageNumber", N_("Sequence Image Number"), N_("Sequence Image Number"), sony2010eId, makerTags, unsignedLong, 1, printValue},
        {4, "SequenceFileNumber", N_("SequenceFileNumber"), N_("SequenceFileNumber"), sony2010eId, makerTags, unsignedLong, 1, printValue},
        {8, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), sony2010eId, makerTags, unsignedLong, 1, printValue},
        {540, "DigitalZoomRatio", N_("DigitalZoomRatio"), N_("DigitalZoomRatio"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {556, "SonyDateTime", N_("SonyDateTime"), N_("SonyDateTime"), sony2010eId, makerTags, undefined, 1, printValue},
        {808, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {1208, "MeterInfo", N_("MeterInfo"), N_("MeterInfo"), sony2010eId, makerTags, undefined, 1, printValue},
        {4444, "ReleaseMode3", N_("ReleaseMode3"), N_("ReleaseMode3"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4448, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4456, "SelfTimer", N_("SelfTimer"), N_("SelfTimer"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4460, "FlashMode", N_("FlashMode"), N_("FlashMode"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4466, "StopsAboveBaseISO", N_("StopsAboveBaseISO"), N_("StopsAboveBaseISO"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4468, "BrightnessValue", N_("BrightnessValue"), N_("BrightnessValue"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4472, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4476, "HDRSetting", N_("HDRSetting"), N_("HDRSetting"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4480, "ExposureCompensation", N_("ExposureCompensation"), N_("ExposureCompensation"), sony2010eId, makerTags, signedShort, 1, printValue},
        {4502, "PictureProfile", N_("PictureProfile"), N_("PictureProfile"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4503, "PictureProfile2", N_("PictureProfile2"), N_("PictureProfile2"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4507, "PictureEffect2", N_("PictureEffect2"), N_("PictureEffect2"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4520, "Quality2", N_("Quality2"), N_("Quality2"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4524, "MeteringMode", N_("MeteringMode"), N_("MeteringMode"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4525, "ExposureProgram", N_("ExposureProgram"), N_("ExposureProgram"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {4532, "WB_RGBLevels", N_("WB_RGBLevels"), N_("WB_RGBLevels"), sony2010eId, makerTags, unsignedShort, 3, printValue},
        {4692, "SonyISO", N_("SonyISO"), N_("SonyISO"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4696, "SonyISO2", N_("SonyISO2"), N_("SonyISO2"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4728, "FocalLength", N_("FocalLength"), N_("FocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4730, "MinFocalLength", N_("MinFocalLength"), N_("MinFocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4732, "MaxFocalLength", N_("MaxFocalLength"), N_("MaxFocalLength"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {4736, "SonyISO3", N_("SonyISO3"), N_("SonyISO3"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {6256, "DistortionCorrParams", N_("DistortionCorrParams"), N_("DistortionCorrParams"), sony2010eId, makerTags, signedShort, 16, printValue},
        {6289, "LensFormat", N_("LensFormat"), N_("LensFormat"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {6290, "LensMount", N_("LensMount"), N_("LensMount"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {6291, "LensType2", N_("LensType2"), N_("LensType2"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {6294, "LensType", N_("LensType"), N_("LensType"), sony2010eId, makerTags, unsignedShort, 1, printValue},
        {6296, "DistortionCorrParamsPresent", N_("DistortionCorrParamsPresent"), N_("DistortionCorrParamsPresent"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        {6297, "DistortionCorrParamsNumber", N_("DistortionCorrParamsNumber"), N_("DistortionCorrParamsNumber"), sony2010eId, makerTags, unsignedByte, 1, printValue},
        // End of list marker
        {0xffff, "(UnknownSony2010eTag)", "(UnknownSony2010eTag)"   , "(UnknownSony2010eTag)"    , sony2010eId, makerTags, unsignedByte, 1, printValue},
    };

    const TagInfo* SonyMakerNote::tagList2010e()
    {
        return tagInfo2010e_;
    }

    // https://github.com/Exiv2/exiv2/pull/906#issuecomment-504338797
    static DataBuf sonyTagCipher(uint16_t /* tag */, const byte* bytes, uint32_t size, TiffComponent* const /*object*/, bool bDecipher)
    {
        DataBuf b(bytes,size); // copy the data

        // initialize the code table
        byte  code[256];
        for ( uint32_t i = 0 ; i < 249 ; i++ ) {
            if ( bDecipher ) {
                code[(i * i * i) % 249] = i ;
            } else {
                code[i] = (i * i * i) % 249 ;
            }
        }
        for ( uint32_t i = 249 ; i < 256 ; i++ ) {
            code[i] = i;
        }

        // code byte-by-byte
        for ( uint32_t i = 0 ; i < size ; i++ ) {
            b.write_uint8(i, code[bytes[i]]);
        }

        return b;
    }

    DataBuf sonyTagDecipher(uint16_t tag, const byte* bytes, uint32_t size, TiffComponent* const object)
    {
        return sonyTagCipher(tag,bytes,size,object,true);
    }
    DataBuf sonyTagEncipher(uint16_t tag, const byte* bytes, uint32_t size, TiffComponent* const object)
    {
        return sonyTagCipher(tag,bytes,size,object,false);
    }

    }  // namespace Internal
}  // namespace Exiv2
