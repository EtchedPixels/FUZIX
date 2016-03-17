#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAXDIM(a)		    (sizeof(a) / sizeof(a[0]))
struct trav {
	short tdest;
	short tverb;
	short tcond;
};
struct travtab {
	struct trav *pTrav;	// trav array for location
	uint8_t sTrav;		// # entries for location
};
static struct trav Trav001[] = { {2, 2, 0}, {2, 44, 0}, {2, 29, 0}, {3, 3, 0}, {3, 12, 0}, {3, 19, 0}, {3, 43, 0}, {4, 5, 0}, {4, 13, 0}, {4, 14, 0}, {4, 46, 0}, {4, 30, 0}, {5, 6, 0}, {5, 45, 0}, {5, 43, 0}, {8, 63, 0},
};
static struct trav Trav002[] = { {1, 2, 0}, {1, 12, 0}, {1, 7, 0}, {1, 43, 0}, {1, 45, 0}, {1, 30, 0}, {5, 6, 0}, {5, 45, 0}, {5, 46, 0},
};
static struct trav Trav003[] = { {1, 3, 0}, {1, 11, 0}, {1, 32, 0}, {1, 44, 0}, {11, 62, 0}, {33, 65, 0}, {79, 5, 0}, {79, 14, 0},
};
static struct trav Trav004[] = { {1, 4, 0}, {1, 12, 0}, {1, 45, 0}, {5, 6, 0}, {5, 43, 0}, {5, 44, 0}, {5, 29, 0}, {7, 5, 0}, {7, 46, 0}, {7, 30, 0}, {8, 63, 0},
};
static struct trav Trav005[] = { {4, 9, 0}, {4, 43, 0}, {4, 30, 0}, {5, 6, 50}, {5, 7, 50}, {5, 45, 50}, {6, 6, 0}, {5, 44, 0}, {5, 46, 0},
};
static struct trav Trav006[] = { {1, 2, 0}, {1, 45, 0}, {4, 9, 0}, {4, 43, 0}, {4, 44, 0}, {4, 30, 0}, {5, 6, 0}, {5, 46, 0},
};
static struct trav Trav007[] = { {1, 12, 0}, {4, 4, 0}, {4, 45, 0}, {5, 6, 0}, {5, 43, 0}, {5, 44, 0}, {8, 5, 0}, {8, 15, 0}, {8, 16, 0}, {8, 46, 0}, {595, 60, 0}, {595, 14, 0}, {595, 30, 0},
};
static struct trav Trav008[] = { {5, 6, 0}, {5, 43, 0}, {5, 46, 0}, {5, 44, 0}, {1, 12, 0}, {7, 4, 0}, {7, 13, 0}, {7, 45, 0}, {9, 3, 303}, {9, 19, 303}, {9, 30, 303}, {593, 3, 0},
};
static struct trav Trav009[] = { {8, 11, 303}, {8, 29, 303}, {593, 11, 0}, {10, 17, 0}, {10, 18, 0}, {10, 19, 0}, {10, 44, 0}, {14, 31, 0}, {11, 51, 0},
};
static struct trav Trav010[] = { {9, 11, 0}, {9, 20, 0}, {9, 21, 0}, {9, 43, 0}, {11, 19, 0}, {11, 22, 0}, {11, 44, 0}, {11, 51, 0}, {14, 31, 0},
};
static struct trav Trav011[] = { {8, 63, 303}, {9, 64, 0}, {10, 17, 0}, {10, 18, 0}, {10, 23, 0}, {10, 24, 0}, {10, 43, 0}, {12, 25, 0}, {12, 19, 0}, {12, 29, 0}, {12, 44, 0}, {3, 62, 0}, {14, 31, 0},
};
static struct trav Trav012[] = { {8, 63, 303}, {9, 64, 0}, {11, 30, 0}, {11, 43, 0}, {11, 51, 0}, {13, 19, 0}, {13, 29, 0}, {13, 44, 0}, {14, 31, 0},
};
static struct trav Trav013[] = { {8, 63, 303}, {9, 64, 0}, {11, 51, 0}, {12, 25, 0}, {12, 43, 0}, {14, 23, 0}, {14, 31, 0}, {14, 44, 0},
};
static struct trav Trav014[] = { {8, 63, 303}, {9, 64, 0}, {11, 51, 0}, {13, 23, 0}, {13, 43, 0}, {20, 30, 150}, {20, 31, 150}, {20, 34, 150}, {15, 30, 0}, {16, 33, 0}, {16, 44, 0},
};
static struct trav Trav015[] = { {18, 36, 0}, {18, 46, 0}, {17, 7, 0}, {17, 38, 0}, {17, 44, 0}, {19, 10, 0}, {19, 30, 0}, {19, 45, 0}, {22, 29, 150}, {22, 31, 150}, {22, 34, 150}, {22, 35, 150}, {22, 23, 150}, {22, 43, 150}, {14, 29, 0}, {34, 55, 0},
};
static struct trav Trav016[] = { {14, 1, 0},
};
static struct trav Trav017[] = { {15, 38, 0}, {15, 43, 0}, {596, 39, 312}, {21, 7, 412}, {597, 41, 412}, {597, 42, 412}, {597, 44, 412}, {597, 69, 412}, {27, 41, 0},
};
static struct trav Trav018[] = { {15, 38, 0}, {15, 11, 0}, {15, 45, 0},
};
static struct trav Trav019[] = { {15, 10, 0}, {15, 29, 0}, {15, 43, 0}, {28, 45, 311}, {28, 36, 311}, {29, 46, 311}, {29, 37, 311}, {30, 44, 311}, {30, 7, 311}, {32, 45, 0}, {74, 49, 35}, {32, 49, 211}, {74, 66, 0},
};
static struct trav Trav020[] = { {0, 1, 0},
};
static struct trav Trav021[] = { {0, 1, 0},
};
static struct trav Trav022[] = { {15, 1, 0},
};
static struct trav Trav023[] = { {67, 43, 0}, {67, 42, 0}, {68, 44, 0}, {68, 61, 0}, {25, 30, 0}, {25, 31, 0}, {648, 52, 0},
};
static struct trav Trav024[] = { {67, 29, 0}, {67, 11, 0},
};
static struct trav Trav025[] = { {23, 29, 0}, {23, 11, 0}, {31, 56, 724}, {26, 56, 0},
};
static struct trav Trav026[] = { {88, 1, 0},
};
static struct trav Trav027[] = { {596, 39, 312}, {21, 7, 412}, {597, 41, 412}, {597, 42, 412}, {597, 43, 412}, {597, 69, 412}, {17, 41, 0}, {40, 45, 0}, {41, 44, 0},
};
static struct trav Trav028[] = { {19, 38, 0}, {19, 11, 0}, {19, 46, 0}, {33, 45, 0}, {33, 55, 0}, {36, 30, 0}, {36, 52, 0},
};
static struct trav Trav029[] = { {19, 38, 0}, {19, 11, 0}, {19, 45, 0},
};
static struct trav Trav030[] = { {19, 38, 0}, {19, 11, 0}, {19, 43, 0}, {62, 44, 0}, {62, 29, 0},
};
static struct trav Trav031[] = { {89, 1, 524}, {90, 1, 0},
};
static struct trav Trav032[] = { {19, 1, 0},
};
static struct trav Trav033[] = { {3, 65, 0}, {28, 46, 0}, {34, 43, 0}, {34, 53, 0}, {34, 54, 0}, {35, 44, 0}, {302, 71, 159}, {100, 71, 0},
};
static struct trav Trav034[] = { {33, 30, 0}, {33, 55, 0}, {15, 29, 0},
};
static struct trav Trav035[] = { {33, 43, 0}, {33, 55, 0}, {20, 39, 0},
};
static struct trav Trav036[] = { {37, 43, 0}, {37, 17, 0}, {28, 29, 0}, {28, 52, 0}, {39, 44, 0}, {65, 70, 0},
};
static struct trav Trav037[] = { {36, 44, 0}, {36, 17, 0}, {38, 30, 0}, {38, 31, 0}, {38, 56, 0},
};
static struct trav Trav038[] = { {37, 56, 0}, {37, 29, 0}, {37, 11, 0}, {595, 60, 0}, {595, 14, 0}, {595, 30, 0}, {595, 4, 0}, {595, 5, 0},
};
static struct trav Trav039[] = { {36, 43, 0}, {36, 23, 0}, {64, 30, 0}, {64, 52, 0}, {64, 58, 0}, {65, 70, 0},
};
static struct trav Trav040[] = { {41, 1, 0},
};
static struct trav Trav041[] = { {42, 46, 0}, {42, 29, 0}, {42, 23, 0}, {42, 56, 0}, {27, 43, 0}, {59, 45, 0}, {60, 44, 0}, {60, 17, 0},
};
static struct trav Trav042[] = { {41, 29, 0}, {42, 45, 0}, {43, 43, 0}, {45, 46, 0}, {80, 44, 0},
};
static struct trav Trav043[] = { {42, 44, 0}, {44, 46, 0}, {45, 43, 0},
};
static struct trav Trav044[] = { {43, 43, 0}, {48, 30, 0}, {50, 46, 0}, {82, 45, 0},
};
static struct trav Trav045[] = { {42, 44, 0}, {43, 45, 0}, {46, 43, 0}, {47, 46, 0}, {87, 29, 0}, {87, 30, 0},
};
static struct trav Trav046[] = { {45, 44, 0}, {45, 11, 0},
};
static struct trav Trav047[] = { {45, 43, 0}, {45, 11, 0},
};
static struct trav Trav048[] = { {44, 29, 0}, {44, 11, 0},
};
static struct trav Trav049[] = { {50, 43, 0}, {51, 44, 0},
};
static struct trav Trav050[] = { {44, 43, 0}, {49, 44, 0}, {51, 30, 0}, {52, 46, 0},
};
static struct trav Trav051[] = { {49, 44, 0}, {50, 29, 0}, {52, 43, 0}, {53, 46, 0},
};
static struct trav Trav052[] = { {50, 44, 0}, {51, 43, 0}, {52, 46, 0}, {53, 29, 0}, {55, 45, 0}, {86, 30, 0},
};
static struct trav Trav053[] = { {51, 44, 0}, {52, 45, 0}, {54, 46, 0},
};
static struct trav Trav054[] = { {53, 44, 0}, {53, 11, 0},
};
static struct trav Trav055[] = { {52, 44, 0}, {55, 45, 0}, {56, 30, 0}, {57, 43, 0},
};
static struct trav Trav056[] = { {55, 29, 0}, {55, 11, 0},
};
static struct trav Trav057[] = { {13, 30, 0}, {13, 56, 0}, {55, 44, 0}, {58, 46, 0}, {83, 45, 0}, {84, 43, 0},
};
static struct trav Trav058[] = { {57, 43, 0}, {57, 11, 0},
};
static struct trav Trav059[] = { {27, 1, 0},
};
static struct trav Trav060[] = { {41, 43, 0}, {41, 29, 0}, {41, 17, 0}, {61, 44, 0}, {62, 45, 0}, {62, 30, 0}, {62, 52, 0},
};
static struct trav Trav061[] = { {60, 43, 0}, {62, 45, 0}, {107, 46, 100},
};
static struct trav Trav062[] = { {60, 44, 0}, {63, 45, 0}, {30, 43, 0}, {61, 46, 0},
};
static struct trav Trav063[] = { {62, 46, 0}, {62, 11, 0},
};
static struct trav Trav064[] = { {39, 29, 0}, {39, 56, 0}, {39, 59, 0}, {65, 44, 0}, {65, 70, 0}, {103, 45, 0}, {103, 74, 0}, {106, 43, 0},
};
static struct trav Trav065[] = { {64, 43, 0}, {66, 44, 0}, {556, 46, 80}, {68, 61, 0}, {556, 29, 80}, {70, 29, 50}, {39, 29, 0}, {556, 45, 60}, {72, 45, 75}, {71, 45, 0}, {556, 30, 80}, {106, 30, 0},
};
static struct trav Trav066[] = { {65, 47, 0}, {67, 44, 0}, {556, 46, 80}, {77, 25, 0}, {96, 43, 0}, {556, 50, 50}, {97, 72, 0},
};
static struct trav Trav067[] = { {66, 43, 0}, {23, 44, 0}, {23, 42, 0}, {24, 30, 0}, {24, 31, 0},
};
static struct trav Trav068[] = { {23, 46, 0}, {69, 29, 0}, {69, 56, 0}, {65, 45, 0},
};
static struct trav Trav069[] = { {68, 30, 0}, {68, 61, 0}, {120, 46, 331}, {119, 46, 0}, {109, 45, 0}, {113, 75, 0},
};
static struct trav Trav070[] = { {71, 45, 0}, {65, 30, 0}, {65, 23, 0}, {111, 46, 0},
};
static struct trav Trav071[] = { {65, 48, 0}, {70, 46, 0}, {110, 45, 0},
};
static struct trav Trav072[] = { {65, 70, 0}, {118, 49, 0}, {73, 45, 0}, {97, 48, 0}, {97, 72, 0},
};
static struct trav Trav073[] = { {72, 46, 0}, {72, 17, 0}, {72, 11, 0},
};
static struct trav Trav074[] = { {19, 43, 0}, {120, 44, 331}, {121, 44, 0}, {75, 30, 0},
};
static struct trav Trav075[] = { {76, 46, 0}, {77, 45, 0},
};
static struct trav Trav076[] = { {75, 45, 0},
};
static struct trav Trav077[] = { {75, 43, 0}, {78, 44, 0}, {66, 45, 0}, {66, 17, 0},
};
static struct trav Trav078[] = { {77, 46, 0},
};
static struct trav Trav079[] = { {3, 1, 0},
};
static struct trav Trav080[] = { {42, 45, 0}, {80, 44, 0}, {80, 46, 0}, {81, 43, 0},
};
static struct trav Trav081[] = { {80, 44, 0}, {80, 11, 0},
};
static struct trav Trav082[] = { {44, 46, 0}, {44, 11, 0},
};
static struct trav Trav083[] = { {57, 46, 0}, {84, 43, 0}, {85, 44, 0},
};
static struct trav Trav084[] = { {57, 45, 0}, {83, 44, 0}, {114, 50, 0},
};
static struct trav Trav085[] = { {83, 43, 0}, {83, 11, 0},
};
static struct trav Trav086[] = { {52, 29, 0}, {52, 11, 0},
};
static struct trav Trav087[] = { {45, 29, 0}, {45, 30, 0},
};
static struct trav Trav088[] = { {25, 30, 0}, {25, 56, 0}, {25, 43, 0}, {20, 39, 0}, {92, 44, 0}, {92, 27, 0},
};
static struct trav Trav089[] = { {25, 1, 0},
};
static struct trav Trav090[] = { {23, 1, 0},
};
static struct trav Trav091[] = { {95, 45, 0}, {95, 73, 0}, {95, 23, 0}, {72, 30, 0}, {72, 56, 0},
};
static struct trav Trav092[] = { {88, 46, 0}, {93, 43, 0}, {94, 45, 0},
};
static struct trav Trav093[] = { {92, 46, 0}, {92, 27, 0}, {92, 11, 0},
};
static struct trav Trav094[] = { {92, 46, 0}, {92, 27, 0}, {92, 23, 0}, {95, 45, 309}, {95, 3, 309}, {95, 73, 309}, {611, 45, 0},
};
static struct trav Trav095[] = { {94, 46, 0}, {94, 11, 0}, {92, 27, 0}, {91, 44, 0},
};
static struct trav Trav096[] = { {66, 44, 0}, {66, 11, 0},
};
static struct trav Trav097[] = { {66, 48, 0}, {72, 44, 0}, {72, 17, 0}, {98, 29, 0}, {98, 45, 0}, {98, 73, 0},
};
static struct trav Trav098[] = { {97, 46, 0}, {97, 72, 0}, {99, 44, 0},
};
static struct trav Trav099[] = { {98, 50, 0}, {98, 73, 0}, {301, 43, 0}, {301, 23, 0}, {100, 43, 0},
};
static struct trav Trav100[] = { {301, 44, 0}, {301, 23, 0}, {301, 11, 0}, {99, 44, 0}, {302, 71, 159}, {33, 71, 0}, {101, 47, 0}, {101, 22, 0},
};
static struct trav Trav101[] = { {100, 46, 0}, {100, 71, 0}, {100, 11, 0},
};
static struct trav Trav102[] = { {103, 30, 0}, {103, 74, 0}, {103, 11, 0},
};
static struct trav Trav103[] = { {102, 29, 0}, {102, 38, 0}, {104, 30, 0}, {618, 46, 114}, {619, 46, 115}, {64, 46, 0},
};
static struct trav Trav104[] = { {103, 29, 0}, {103, 74, 0}, {105, 30, 0},
};
static struct trav Trav105[] = { {104, 29, 0}, {104, 11, 0}, {103, 74, 0},
};
static struct trav Trav106[] = { {64, 29, 0}, {65, 44, 0}, {108, 43, 0},
};
static struct trav Trav107[] = { {131, 46, 0}, {132, 49, 0}, {133, 47, 0}, {134, 48, 0}, {135, 29, 0}, {136, 50, 0}, {137, 43, 0}, {138, 44, 0}, {139, 45, 0}, {61, 30, 0},
};
static struct trav Trav108[] = { {556, 43, 95}, {556, 45, 95}, {556, 46, 95}, {556, 47, 95}, {556, 48, 95}, {556, 49, 95}, {556, 50, 95}, {556, 29, 95}, {556, 30, 95}, {106, 43, 0}, {626, 44, 0},
};
static struct trav Trav109[] = { {69, 46, 0}, {113, 45, 0}, {113, 75, 0},
};
static struct trav Trav110[] = { {71, 44, 0}, {20, 39, 0},
};
static struct trav Trav111[] = { {70, 45, 0}, {50, 30, 40}, {50, 39, 40}, {50, 56, 40}, {53, 30, 50}, {45, 30, 0},
};
static struct trav Trav112[] = { {131, 49, 0}, {132, 45, 0}, {133, 43, 0}, {134, 50, 0}, {135, 48, 0}, {136, 47, 0}, {137, 44, 0}, {138, 30, 0}, {139, 29, 0}, {140, 46, 0},
};
static struct trav Trav113[] = { {109, 46, 0}, {109, 11, 0}, {109, 109, 0},
};
static struct trav Trav114[] = { {84, 48, 0},
};
static struct trav Trav115[] = { {116, 49, 0},
};
static struct trav Trav116[] = { {115, 47, 0}, {593, 30, 0},
};
static struct trav Trav117[] = { {118, 49, 0}, {660, 41, 233}, {660, 42, 233}, {660, 69, 233}, {660, 47, 233}, {661, 41, 332}, {303, 41, 0}, {21, 39, 332}, {596, 39, 0},
};
static struct trav Trav118[] = { {72, 30, 0}, {117, 29, 0},
};
static struct trav Trav119[] = { {69, 45, 0}, {69, 11, 0}, {653, 43, 0}, {653, 7, 0},
};
static struct trav Trav120[] = { {69, 45, 0}, {74, 43, 0},
};
static struct trav Trav121[] = { {74, 43, 0}, {74, 11, 0}, {653, 45, 0}, {653, 7, 0},
};
static struct trav Trav122[] = { {123, 47, 0}, {660, 41, 233}, {660, 42, 233}, {660, 69, 233}, {660, 49, 233}, {303, 41, 0}, {596, 39, 0}, {124, 77, 0}, {126, 28, 0}, {129, 40, 0},
};
static struct trav Trav123[] = { {122, 44, 0}, {124, 43, 0}, {124, 77, 0}, {126, 28, 0}, {129, 40, 0},
};
static struct trav Trav124[] = { {123, 44, 0}, {125, 47, 0}, {125, 36, 0}, {128, 48, 0}, {128, 37, 0}, {128, 30, 0}, {126, 28, 0}, {129, 40, 0},
};
static struct trav Trav125[] = { {124, 46, 0}, {124, 77, 0}, {126, 45, 0}, {126, 28, 0}, {127, 43, 0}, {127, 17, 0},
};
static struct trav Trav126[] = { {125, 46, 0}, {125, 23, 0}, {125, 11, 0}, {124, 77, 0}, {610, 30, 0}, {610, 39, 0},
};
static struct trav Trav127[] = { {125, 44, 0}, {125, 11, 0}, {125, 17, 0}, {124, 77, 0}, {126, 28, 0},
};
static struct trav Trav128[] = { {124, 45, 0}, {124, 29, 0}, {124, 77, 0}, {129, 46, 0}, {129, 30, 0}, {129, 40, 0}, {126, 28, 0},
};
static struct trav Trav129[] = { {128, 44, 0}, {128, 29, 0}, {124, 77, 0}, {130, 43, 0}, {130, 19, 0}, {130, 40, 0}, {130, 3, 0}, {126, 28, 0},
};
static struct trav Trav130[] = { {129, 44, 0}, {124, 77, 0}, {126, 28, 0},
};
static struct trav Trav131[] = { {107, 44, 0}, {132, 48, 0}, {133, 50, 0}, {134, 49, 0}, {135, 47, 0}, {136, 29, 0}, {137, 30, 0}, {138, 45, 0}, {139, 46, 0}, {112, 43, 0},
};
static struct trav Trav132[] = { {107, 50, 0}, {131, 29, 0}, {133, 45, 0}, {134, 46, 0}, {135, 44, 0}, {136, 49, 0}, {137, 47, 0}, {138, 43, 0}, {139, 30, 0}, {112, 48, 0},
};
static struct trav Trav133[] = { {107, 29, 0}, {131, 30, 0}, {132, 44, 0}, {134, 47, 0}, {135, 49, 0}, {136, 43, 0}, {137, 45, 0}, {138, 50, 0}, {139, 48, 0}, {112, 46, 0},
};
static struct trav Trav134[] = { {107, 47, 0}, {131, 45, 0}, {132, 50, 0}, {133, 48, 0}, {135, 43, 0}, {136, 30, 0}, {137, 46, 0}, {138, 29, 0}, {139, 44, 0}, {112, 49, 0},
};
static struct trav Trav135[] = { {107, 45, 0}, {131, 48, 0}, {132, 30, 0}, {133, 46, 0}, {134, 43, 0}, {136, 44, 0}, {137, 49, 0}, {138, 47, 0}, {139, 50, 0}, {112, 29, 0},
};
static struct trav Trav136[] = { {107, 43, 0}, {131, 44, 0}, {132, 29, 0}, {133, 49, 0}, {134, 30, 0}, {135, 46, 0}, {137, 50, 0}, {138, 48, 0}, {139, 47, 0}, {112, 45, 0},
};
static struct trav Trav137[] = { {107, 48, 0}, {131, 47, 0}, {132, 46, 0}, {133, 30, 0}, {134, 29, 0}, {135, 50, 0}, {136, 45, 0}, {138, 49, 0}, {139, 43, 0}, {112, 44, 0},
};
static struct trav Trav138[] = { {107, 30, 0}, {131, 43, 0}, {132, 47, 0}, {133, 29, 0}, {134, 44, 0}, {135, 45, 0}, {136, 46, 0}, {137, 48, 0}, {139, 49, 0}, {112, 50, 0},
};
static struct trav Trav139[] = { {107, 49, 0}, {131, 50, 0}, {132, 43, 0}, {133, 44, 0}, {134, 45, 0}, {135, 30, 0}, {136, 48, 0}, {137, 29, 0}, {138, 46, 0}, {112, 47, 0},
};
static struct trav Trav140[] = { {112, 45, 0}, {112, 11, 0},
};

struct travtab TravTab[] = {
	{Trav001, MAXDIM(Trav001)},
	{Trav002, MAXDIM(Trav002)},
	{Trav003, MAXDIM(Trav003)},
	{Trav004, MAXDIM(Trav004)},
	{Trav005, MAXDIM(Trav005)},
	{Trav006, MAXDIM(Trav006)},
	{Trav007, MAXDIM(Trav007)},
	{Trav008, MAXDIM(Trav008)},
	{Trav009, MAXDIM(Trav009)},
	{Trav010, MAXDIM(Trav010)},
	{Trav011, MAXDIM(Trav011)},
	{Trav012, MAXDIM(Trav012)},
	{Trav013, MAXDIM(Trav013)},
	{Trav014, MAXDIM(Trav014)},
	{Trav015, MAXDIM(Trav015)},
	{Trav016, MAXDIM(Trav016)},
	{Trav017, MAXDIM(Trav017)},
	{Trav018, MAXDIM(Trav018)},
	{Trav019, MAXDIM(Trav019)},
	{Trav020, MAXDIM(Trav020)},
	{Trav021, MAXDIM(Trav021)},
	{Trav022, MAXDIM(Trav022)},
	{Trav023, MAXDIM(Trav023)},
	{Trav024, MAXDIM(Trav024)},
	{Trav025, MAXDIM(Trav025)},
	{Trav026, MAXDIM(Trav026)},
	{Trav027, MAXDIM(Trav027)},
	{Trav028, MAXDIM(Trav028)},
	{Trav029, MAXDIM(Trav029)},
	{Trav030, MAXDIM(Trav030)},
	{Trav031, MAXDIM(Trav031)},
	{Trav032, MAXDIM(Trav032)},
	{Trav033, MAXDIM(Trav033)},
	{Trav034, MAXDIM(Trav034)},
	{Trav035, MAXDIM(Trav035)},
	{Trav036, MAXDIM(Trav036)},
	{Trav037, MAXDIM(Trav037)},
	{Trav038, MAXDIM(Trav038)},
	{Trav039, MAXDIM(Trav039)},
	{Trav040, MAXDIM(Trav040)},
	{Trav041, MAXDIM(Trav041)},
	{Trav042, MAXDIM(Trav042)},
	{Trav043, MAXDIM(Trav043)},
	{Trav044, MAXDIM(Trav044)},
	{Trav045, MAXDIM(Trav045)},
	{Trav046, MAXDIM(Trav046)},
	{Trav047, MAXDIM(Trav047)},
	{Trav048, MAXDIM(Trav048)},
	{Trav049, MAXDIM(Trav049)},
	{Trav050, MAXDIM(Trav050)},
	{Trav051, MAXDIM(Trav051)},
	{Trav052, MAXDIM(Trav052)},
	{Trav053, MAXDIM(Trav053)},
	{Trav054, MAXDIM(Trav054)},
	{Trav055, MAXDIM(Trav055)},
	{Trav056, MAXDIM(Trav056)},
	{Trav057, MAXDIM(Trav057)},
	{Trav058, MAXDIM(Trav058)},
	{Trav059, MAXDIM(Trav059)},
	{Trav060, MAXDIM(Trav060)},
	{Trav061, MAXDIM(Trav061)},
	{Trav062, MAXDIM(Trav062)},
	{Trav063, MAXDIM(Trav063)},
	{Trav064, MAXDIM(Trav064)},
	{Trav065, MAXDIM(Trav065)},
	{Trav066, MAXDIM(Trav066)},
	{Trav067, MAXDIM(Trav067)},
	{Trav068, MAXDIM(Trav068)},
	{Trav069, MAXDIM(Trav069)},
	{Trav070, MAXDIM(Trav070)},
	{Trav071, MAXDIM(Trav071)},
	{Trav072, MAXDIM(Trav072)},
	{Trav073, MAXDIM(Trav073)},
	{Trav074, MAXDIM(Trav074)},
	{Trav075, MAXDIM(Trav075)},
	{Trav076, MAXDIM(Trav076)},
	{Trav077, MAXDIM(Trav077)},
	{Trav078, MAXDIM(Trav078)},
	{Trav079, MAXDIM(Trav079)},
	{Trav080, MAXDIM(Trav080)},
	{Trav081, MAXDIM(Trav081)},
	{Trav082, MAXDIM(Trav082)},
	{Trav083, MAXDIM(Trav083)},
	{Trav084, MAXDIM(Trav084)},
	{Trav085, MAXDIM(Trav085)},
	{Trav086, MAXDIM(Trav086)},
	{Trav087, MAXDIM(Trav087)},
	{Trav088, MAXDIM(Trav088)},
	{Trav089, MAXDIM(Trav089)},
	{Trav090, MAXDIM(Trav090)},
	{Trav091, MAXDIM(Trav091)},
	{Trav092, MAXDIM(Trav092)},
	{Trav093, MAXDIM(Trav093)},
	{Trav094, MAXDIM(Trav094)},
	{Trav095, MAXDIM(Trav095)},
	{Trav096, MAXDIM(Trav096)},
	{Trav097, MAXDIM(Trav097)},
	{Trav098, MAXDIM(Trav098)},
	{Trav099, MAXDIM(Trav099)},
	{Trav100, MAXDIM(Trav100)},
	{Trav101, MAXDIM(Trav101)},
	{Trav102, MAXDIM(Trav102)},
	{Trav103, MAXDIM(Trav103)},
	{Trav104, MAXDIM(Trav104)},
	{Trav105, MAXDIM(Trav105)},
	{Trav106, MAXDIM(Trav106)},
	{Trav107, MAXDIM(Trav107)},
	{Trav108, MAXDIM(Trav108)},
	{Trav109, MAXDIM(Trav109)},
	{Trav110, MAXDIM(Trav110)},
	{Trav111, MAXDIM(Trav111)},
	{Trav112, MAXDIM(Trav112)},
	{Trav113, MAXDIM(Trav113)},
	{Trav114, MAXDIM(Trav114)},
	{Trav115, MAXDIM(Trav115)},
	{Trav116, MAXDIM(Trav116)},
	{Trav117, MAXDIM(Trav117)},
	{Trav118, MAXDIM(Trav118)},
	{Trav119, MAXDIM(Trav119)},
	{Trav120, MAXDIM(Trav120)},
	{Trav121, MAXDIM(Trav121)},
	{Trav122, MAXDIM(Trav122)},
	{Trav123, MAXDIM(Trav123)},
	{Trav124, MAXDIM(Trav124)},
	{Trav125, MAXDIM(Trav125)},
	{Trav126, MAXDIM(Trav126)},
	{Trav127, MAXDIM(Trav127)},
	{Trav128, MAXDIM(Trav128)},
	{Trav129, MAXDIM(Trav129)},
	{Trav130, MAXDIM(Trav130)},
	{Trav131, MAXDIM(Trav131)},
	{Trav132, MAXDIM(Trav132)}, {Trav133, MAXDIM(Trav133)}, {Trav134, MAXDIM(Trav134)}, {Trav135, MAXDIM(Trav135)}, {Trav136, MAXDIM(Trav136)}, {Trav137, MAXDIM(Trav137)}, {Trav138, MAXDIM(Trav138)}, {Trav139, MAXDIM(Trav139)}, {Trav140,
																													 MAXDIM(Trav140)},
};

char *pObjDesc[] = {
/*  Object 1  */
	"Set of keys." "/There are some keys on the ground here.",
/*  Object 2  */
	"Brass lantern" "/There is a shiny brass lamp nearby." "/There is a lamp shining nearby.",
/*  Object 3  */
	"*Grate" "/The grate is locked." "/The grate is open.",
/*  Object 4  */
	"Wicker cage" "/There is a small wicker cage discarded nearby.",
/*  Object 5  */
	"Black rod" "/A three foot black rod with a rusty star on an end lies nearby.",
/*  Object 6  */
	"Black rod" "/A three foot black rod with a rusty mark on an end lies nearby.",
/*  Object 7  */
	"*Steps" "/Rough stone steps lead down the pit." "/Rough stone steps lead up the dome.",
/*  Object 8  */
	"Little bird in cage" "/A cheerful little bird is sitting here singing." "/There is a little bird in the cage.",
/*  Object 9  */
	"*Rusty door" "/The way north is barred by a massive, rusty, iron door." "/The way north leads through a massive, rusty, iron door.",
/*  Object 10  */
	"Velvet pillow" "/A small velvet pillow lies on the floor.",
/*  Object 11  */
	"*Snake" "/A huge green fierce snake bars the way!",
/*  Object 12  */
	"*Fissure" "//A crystal bridge now spans the fissure." "/The crystal bridge has vanished!",
/*  Object 13  */
	"*Stone tablet" "/A massive stone tablet imbedded in the wall reads:" "\"Congratulations on bringing light into the dark-room!\"",
/*  Object 14  */
	"Giant clam >Grunt!<" "/There is an enormous clam here with its shell tightly closed.",
/*  Object 15  */
	"Giant oyster >Groan!<" "/There is an enormous oyster here with its shell tightly closed." "/Interesting.  There seems to be something written on the underside of the\n" "oyster.",
/*  Object 16  */
	"\"Spelunker Today\"" "/There are a few recent issues of \"Spelunker Today\" magazine here.",
/*  Object 17  */
	"",
/*  Object 18  */
	"",
/*  Object 19  */
	"Tasty food" "/There is tasty food here.",
/*  Object 20  */
	"Small bottle" "/There is a bottle of water here." "/There is an empty bottle here." "/There is a bottle of oil here.",
/*  Object 21  */
	"Water in the bottle.",
/*  Object 22  */
	"Oil in the bottle",
/*  Object 23  */
	"*Mirror",
/*  Object 24  */
	"*Plant"
	    "/There is a tiny little plant in the pit, murmuring \"Water, Water, ...\""
	    "/The plant spurts into furious growth for a few seconds."
	    "/There is a 12-foot-tall beanstalk stretching up out of the pit, bellowing\n"
	    "\"Water!! Water!!\"" "/The plant grows explosively, almost filling the bottom of the pit." "/There is a gigantic beanstalk stretching all the way up to the hole." "/You've over-watered the plant!  It's shriveling up! It's, It's...",
/*  Object 25  */
	"*Phony plant" "/" "/The top of a 12-foot-tall beanstalk is poking up out of the west pit." "/There is a huge beanstalk growing out of the west pit up to the hole.",
/*  Object 26  */
	"*Stalactite",
/*  Object 27  */
	"*Shadowy figure" "/The shadowy figure seems to be trying to attract your attention.",
/*  Object 28  */
	"Dwarf's axe" "/There is a little axe here." "/There is a little axe lying beside the bear.",
/*  Object 29  */
	"*Cave drawings",
/*  Object 30  */
	"*Pirate",
/*  Object 31  */
	"*Dragon" "/A huge green fierce dragon bars the way!" "/Congratulations!  You have just vanquished a dragon with your bare hands!\n" "(Unbelievable, Isn't it?)" "/The body of a huge green dead dragon is lying off to one side.",
/*  Object 32  */
	"*Chasm"
	    "/A rickety wooden bridge extends across the chasm, vanishing into the mist.\n"
	    "A sign posted on the bridge reads:\n" "                  \"Stop!  Pay Troll!\"" "/The wreckage of a bridge (and a dead bear) can be seen at the bottom of the\n" "chasm.",
/*  Object 33  */
	"*Troll" "/A burly troll stands by the bridge and insists you throw him a treasure\n" "before you may cross." "/The troll steps out from beneath the bridge and blocks your way.",
/*  Object 34  */
	"*Phony troll" "/The troll is nowhere to be seen.",
/*  Object 35  */
	"/There is a ferocious cave bear eyeing you from the far end of the room!" "/There is a gentle cave bear sitting placidly in one corner." "/There is a contented-looking bear wandering about nearby.",
/*  Object 36  */
	"*Message in second maze" "/There is a message scrawled in the dust in a flowery script, reading:\n" "           \"This is not the maze where the\"" "           \"pirate leaves his treasure chest\"",
/*  Object 37  */
	"*Volcano and,or Geyser",
/*  Object 38  */
	"*Vending machine" "/There is a massive vending machine here.  The instructions on it read:\n" "\n" "     \"Drop coins here to receive fresh batteries.\"",
/*  Object 39  */
	"Batteries" "/There are fresh batteries here." "/Some worn-out batteries have been discarded nearby.",
/*  Object 40  */
	"*Carpet and,or moss",
/*  Object 41  */
	"",
/*  Object 42  */
	"",
/*  Object 43  */
	"",
/*  Object 44  */
	"",
/*  Object 45  */
	"",
/*  Object 46  */
	"",
/*  Object 47  */
	"",
/*  Object 48  */
	"",
/*  Object 49  */
	"",
/*  Object 50  */
	"Large gold nugget" "/There is a large sparkling nugget of gold here!",
/*  Object 51  */
	"Several diamonds" "/There are diamonds here!",
/*  Object 52  */
	"Bars of silver" "/There are bars of silver here!",
/*  Object 53  */
	"Precious jewelry" "/There is precious jewelry here!",
/*  Object 54  */
	"Rare coins" "/There are many coins here!",
/*  Object 55  */
	"Treasure chest" "/The pirate's treasure chest is here!",
/*  Object 56  */
	"Golden eggs" "/There is a large nest here, full of golden eggs!" "/The nest of golden eggs has vanished!" "/Done!",
/*  Object 57  */
	"Jeweled trident" "/There is a jewel-encrusted trident here!",
/*  Object 58  */
	"Ming vase" "/There is a delicate, precious, ming vase here!" "/The vase is now resting, delicately, on a velvet pillow." "/The floor is littered with worthless shards of pottery." "/The ming vase drops with a delicate crash.",
/*  Object 59  */
	"Egg-sized emerald" "/There is an emerald here the size of a plover's egg!",
/*  Object 60  */
	"Platinum pyramid" "/There is a platinum pyramid here, 8 inches on a side!",
/*  Object 61  */
	"Glistening pearl" "/Off to one side lies a glistening pearl!",
/*  Object 62  */
	"Persian rug" "/There is a persian rug spread out on the floor!" "/The dragon is sprawled out on a persian rug!!",
/*  Object 63  */
	"Rare spices" "/There are rare spices here!",
/*  Object 64  */
	"Golden chain" "/There is a golden chain lying in a heap on the floor!" "/The bear is locked to the wall with a golden chain!" "/There is a golden chain locked to the wall!",
};

char *pLongRmDesc[] = {
/*  Room 1  */
	"You are standing at the end of a road before a small brick building.  Around\n" "you is a forest.  A small stream flows out of the building and down a gully.",
/*  Room 2  */
	"You have walked up a hill, still in the forest.  The road slopes back down\n" "the other side of the hill.  There is a building in the distance.",
/*  Room 3  */
	"You are inside a building, a well house for a large spring.",
/*  Room 4  */
	"You are in a valley in the forest beside a stream tumbling along a rocky\n" "bed.",
/*  Room 5  */
	"You are in open forest, with a deep valley to one side.",
/*  Room 6  */
	"You are in open forest near both a valley and a road.",
/*  Room 7  */
	"At your feet all the water of the stream splashes into a 2-inch slit in the\n" "rock.  Downstream the streambed is bare rock.",
/*  Room 8  */
	"You are in a 20-foot depression floored with bare dirt.  Set into the dirt\n" "is a strong steel grate mounted in concrete.  A dry streambed leads into the\n" "depression.",
/*  Room 9  */
	"You are in a small chamber beneath a 3x3 steel grate to the surface.  A low\n" "crawl over cobbles leads inward to the West.",
/* Room 10 */
	"You are crawling over cobbles in a low passage.  There is a dim light at the\n" "east end of the passage.",
/*  Room 11  */
	"You are in a debris room filled with stuff washed in from the surface.  A\n"
	    "low wide passage with cobbles becomes plugged with mud and debris here, but\n" "an awkward canyon leads upward and west.  A note on the wall says:\n" "                          Magic Word \"XYZZY\"",
/*  Room 12  */
	"You are in an awkward sloping east/west canyon.",
/*  Room 13  */
	"You are in a splendid chamber thirty feet high.  The walls are frozen rivers\n" "of orange stone.  An awkward canyon and a good passage exit from east and\n" "west sides of the chamber.",
/*  Room 14  */
	"At your feet is a small pit breathing traces of white mist.  An east passage\n" "ends here except for a small crack leading on.",
/*  Room 15  */
	"You are at one end of a vast hall stretching forward out of sight to the\n"
	    "west.  There are openings to either side.  Nearby, a wide stone staircase\n"
	    "leads downward.  The hall is filled with wisps of white mist swaying to and\n" "fro almost as if alive.  A cold wind blows up the staircase.  There is a\n" "passage at the top of a dome behind you.",
/*  Room 16  */
	"The crack is far too small for you to follow.",
/*  Room 17  */
	"You are on the east bank of a fissure slicing clear across the hall.  The\n" "mist is quite thick here, and the fissure is too wide to jump.",
/*  Room 18  */
	"This is a low room with a crude note on the wall.  The note says:\n" "       You won't get it up the steps.",
/*  Room 19  */
	"You are in the hall of the mountain king, with passages off in all\n" "directions.",
/*  Room 20  */
	"You are at the bottom of the pit with a broken neck.",
/*  Room 21  */
	"You didn't make it.",
/*  Room 22  */
	"The dome is unclimbable.",
/*  Room 23  */
	"You are at the west end of the twopit room.  There is a large hole in the\n" "wall above the pit at this end of the room.",
/*  Room 24  */
	"You are that the bottom of the eastern pit in the twopit room.  There is a\n" "small pool of oil in one corner of the pit.",
/*  Room 25  */
	"You are at the bottom of the western pit in the towpit room.  There is a\n" "large hole in the wall about 25 feet above you.",
/*  Room 26  */
	"You clamber up the plant and scurry through the hole at the top.",
/*  Room 27  */
	"You are on the west side of the fissure in the hall of mists.",
/*  Room 28  */
	"You are in a low N/S passage at a hole in the floor.  The hole goes down to\n" "an E/W passage.",
/*  Room 29  */
	"You are in the south side chamber.",
/*  Room 30  */
	"You are in the west side chamber of the hall of the mountain king.  A\n" "passage continues west and up here.",
/*  Room 31  */
	">$<",
/*  Room 32  */
	"You can't get by the snake.",
/*  Room 33  */
	"You are in a large room, with a passage to the south, a passage to the west,\n" "and a wall of broken rock to the east.  There is a large \"Y2\" on a rock in\n" "the room's center.",
/*  Room 34  */
	"You are in a jumble of rock, with cracks everywhere.",
/*  Room 35  */
	"You're at a low window overlooking a huge pit, which extends up out of\n"
	    "sight.  A floor is indistinctly visible over 50 feet below.  Traces of white\n"
	    "mist cover the floor of the pit, becoming thicker to the right.  Marks in\n"
	    "the dust around the window would seem to indicate that someone has been here\n"
	    "recently.  Directly across the pit from you and 25 feet away there is a\n" "similar window looking into a lighted room.  A shadowy figure can be seen\n" "there peering back at you.",
/*  Room 36  */
	"You are in a dirty broken passage.  To the east is a crawl.  To the west is\n" "a large passage.  Above you is another passage.",
/*  Room 37  */
	"You are on the brink of a small clean climbable pit.  A crawl leads west.",
/*  Room 38  */
	"You are in the bottom of a small pit with a little stream, which enters and\n" "exits through tiny slits.",
/*  Room 39  */
	"You are in a large room full of dusty rocks.  There is a big hole in the\n" "floor.  There are cracks everywhere, and a passage leading east.",
/*  Room 40  */
	"You have crawled through a very low wide passage parallel to and north of\n" "the hall of mists.",
/*  Room 41  */
	"You are at the west end of hall of mists.  A low wide crawl continues west\n" "and another goes north.  To the south is a little passage 6 feet off the\n" "floor.",
/*  Room 42  */
	"You are in a maze of twisty little passages, all alike.",
/*  Room 43  */
	"@42",
/*  Room 44  */
	"@42",
/*  Room 45  */
	"@42",
/*  Room 46  */
	"Dead end.",
/*  Room 47  */
	"@46",
/*  Room 48  */
	"@46",
/*  Room 49  */
	"@42",
/*  Room 50  */
	"@42",
/*  Room 51  */
	"@42",
/*  Room 52  */
	"@42",
/*  Room 53  */
	"@42",
/*  Room 54  */
	"@46",
/*  Room 55  */
	"@42",
/*  Room 56  */
	"@46",
/*  Room 57  */
	"You are on the brink of a thirty foot pit with a massive orange column down\n" "one wall.  You could climb down here but you could not get back up.  The\n" "maze continues at this level.",
/*  Room 58  */
	"@46",
/*  Room 59  */
	"You have crawled through a very low wide passage paralled to and north of\n" "the hall of mists.",
/*  Room 60  */
	"You are at the east end of a very long hall apparently without side\n" "chambers.  To the east a low wide crawl slants up.  To the north a round two\n" "foot hole slants down.",
/*  Room 61  */
	"You are at the west end of a very long featureless hall.  The hall joins up\n" "with a narrow north/south passage.",
/*  Room 62  */
	"You are at a crossover of a high N/S passage and a low E/W one.",
/*  Room 63  */
	"@46",
/*  Room 64  */
	"You are at a complex junction.  A low hands and knees passage from the north\n" "joins a higher crawl from the east to make a walking passage going west.\n" "There is also a large room above.  The air is damp here.",
/*  Room 65  */
	"You are in bedquilt, a long east/west passage with holes everywhere.  To\n" "explore at random select north, south, up or down.",
/*  Room 66  */
	"You are in a room whose walls resemble swiss cheese. Obvious passages go\n" "west, east, ne, and nw.  Part of the room is occupied by a large bedrock\n" "block.",
/*  Room 67  */
	"You are at the east end of the twopit room.  The floor here is littered with\n"
	    "thin rock slabs, which make it easy to descend the pits.  There is a path\n"
	    "here bypassing the pits to connect passages from east and west.  There are\n" "holes all over, but the only bit one is on the wall directly over the west\n" "pit where you can't get at it.",
/*  Room 68  */
	"You are in a large low circular chamber whose floor is an immense slab\n"
	    "fallen from the ceiling (slab room). East and west there once were large\n" "passages, but they are now filled with boulders.  Low small passages go\n" "north and south, and the south one quickly bends west around the boulders.",
/*  Room 69  */
	"You are in a secret N/S canyon above a large room.",
/*  Room 70  */
	"You are in a secret N/S canyon above a sizable passage.",
/*  Room 71  */
	"You are in a secret canyon at a junction of three canyons, bearing north,\n" "south and se.  The north one is as tall as the other two combined.",
/*  Room 72  */
	"You are in a large low room.  Crawls lead north, se, and sw.",
/*  Room 73  */
	"Dead end crawl.",
/*  Room 74  */
	"You are in a secret canyon which here runs E/W.  It crosses over a very\n" "tight canyon 15 feet below.  If you go down you may not be able to get back\n" "up.",
/*  Room 75  */
	"You are at a wide place in a very tight N/S canyon.",
/*  Room 76  */
	"The canyon here becomes too tight to go further south.",
/*  Room 77  */
	"You are in a tall E/W canyon.  A low tight crawl goes 3 feet north and seems\n" "to open up.",
/*  Room 78  */
	"The canyon runs into a mass of boulders -- dead end.",
/*  Room 79  */
	"The stream flows out through a pair of 1 foot diameter sewer pipes.  It\n" "would be advisable to use the exit.",
/*  Room 80  */
	"@42",
/*  Room 81  */
	"@46",
/*  Room 82  */
	"@46",
/*  Room 83  */
	"@42",
/*  Room 84  */
	"@42",
/*  Room 85  */
	"@46",
/*  Room 86  */
	"@46",
/*  Room 87  */
	"@42",
/*  Room 88  */
	"You are in a long, narrow corridor stretching out of sight to the west.  At\n" "the eastern end is a hole through which you can see a profusion of leaves,\n",
/*  Room 89  */
	"There is nothing here to climb.  Use \"up\" or \"out\" to leave\n", "the pit.",
/*  Room 90  */
	"You have climbed up the plant and out of the pit.",
/*  Room 91  */
	"You are at the top of a steep incline above a large room. You could climb\n" "down here, but you would not be able to climb up.  There is a passage\n" "leading back to the north.",
/*  Room 92  */
	"You are in the giant room.  The ceiling is too high up for your lamp to show\n" "it.  Cavernous passages lead east, north, and south.  On the west wall is\n" "scrawled the inscription:\n" "              \"Fee Fie Foe Foo\"       {sic}",
/*  Room 93  */
	"The passage here is blocked by a recent cave-in.",
/*  Room 94  */
	"You are at one end of an immense north/south passage.",
/*  Room 95  */
	"You are in a magnificent cavern with a rushing stream, which cascades over a\n" "sparkling waterfall into a roaring whirlpool which disappears through a hole\n" "in the floor.  Passages exit to the south and west.",
/*  Room 96  */
	"You are in the soft room.  The walls are covered with heavy curtains, the\n" "floor with a thick pile carpet. Moss covers the ceiling.",
/*  Room 97  */
	"This is the oriental room.  Ancient oriental cave drawings cover the walls.\n" "A gently sloping passage leads upward to the north, another passage leads se,\n" "and a hands and knees crawl leads west.",
/*  Room 98  */
	"You are following a wide path around the outer edge of a large cavern.  Far\n"
	    "below, through a heavy white mist, strange splashing noises can be heard.\n" "The mist rises up through a fissure in the ceiling.  The path exits to the\n" "south and west.",
/*  Room 99  */
	"You are in an alcove.  A small nw path seems to widen after a short\n" "distance.  An extremely tight tunnel leads east.  It looks like a very tight\n" "squeeze.  An eerie light can be seen at the other end.",
/*  Room 100  */
	"You're in a small chamber lit by an eerie green light.  An extremely narrow\n" "tunnel exits to the west.  A dark corridor leads ne.",
/*  Room 101  */
	"You're in the dark-room.  A corridor leading south is the only exit.",
/*  Room 102  */
	"You are in an arched hall.  A coral passage once continued up and east from\n" "here, but is now blocked by debris.  The air smells of sea water.",
/*  Room 103  */
	"You're in a large room carved out of sedimentary rock.  The floor and walls\n"
	    "are littered with bits of shells imbedded in the stone.  A shallow passage\n" "proceeds downward, and a somewhat steeper one leads up.  A low hands and\n" "knees passage enters from the south.",
/*  Room 104  */
	"You are in a long sloping corridor with ragged sharp walls.",
/*  Room 105  */
	"You are in a cul-de-sac about eight feet across.",
/*  Room 106  */
	"You are in an anteroom leading to a large passage to the east.  Small\n"
	    "passages go west and up.  The remnants of recent digging are evident.  A\n"
	    "sign in midair here says:\n" "            \"Cave under construction beyond this point.\"\n" "                   \"Proceed at your own risk.\"\n" "                  \"Witt construction company\"",
/*  Room 107  */
	"You are in a maze of twisty little passages, all different.",
/*  Room 108  */
	"You are at Witt's end.  Passages lead off in ALL directions.",
/*  Room 109  */
	"You are in a north/south canyon about 25 feet across.  The floor is covered\n"
	    "by white mist seeping in from the north. The walls extend upward for well\n"
	    "over 100 feet.  Suspended from some unseen point far above you, an enormous\n"
	    "two-sided mirror is hanging paralled to and midway between the canyon walls.\n"
	    "(The mirror is obviously provided for the use of the dwarves, who as you\n" "know, are extremely vain.)  A small window can be seen in either wall, some\n" "fifty feet up.",
/*  Room 110  */
	"You're at a low window overlooking a huge pit, which extends up out of\n"
	    "sight.  A floor is indistinctly visible over 50 feet below.  Traces of white\n"
	    "mist cover the floor of the pit, becoming thicker to the left.  Marks in the\n"
	    "dust around the window would seem to indicate that someone has been here\n"
	    "recently.  Directly across the pit from you and 25 feet away there is a\n" "similar window looking into a lighted room.  A shadowy figure can be seen\n" "there peering back at you.",
/*  Room 111  */
	"A large stalactite extends from the roof and almost reaches the floor below.\n" "You could climb down it, and jump from it to the floor, but having done so\n" "you would be unable to reach it to climb back up.",
/*  Room 112  */
	"You are in a little maze of twisting passages, all different.",
/*  Room 113  */
	"You are at the edge of a large underground reservoir.  An opaque cloud of\n"
	    "white mist fills the room and rises rapidly upward.  The lake is fed by a\n"
	    "stream which tumbles out of a hole in the wall about 10 feet overhead and\n" "splashes noisily into the water somewhere within the mist. The only passage\n" "goes back toward the south.",
/*  Room 114  */
	"@46",
/*  Room 115  */
	"@141,142",
/*  Room 116  */
	"@143,144",
/*  Room 117  */
	"You are on one side of a large deep chasm.  A heavy white mist rising up\n" "from below obscures all view of the far side.  A sw path leads away from the\n" "chasm into a winding corridor.",
/*  Room 118  */
	"You are in a long winding corridor sloping out of sight in both directions.",
/*  Room 119  */
	"You are in a secret canyon which exits to the north and east.",
/*  Room 120  */
	"You are in a secret canyon which exits to the north and east.",
/*  Room 121  */
	"You are in a secret canyon which exits to the north and east.",
/*  Room 122  */
	"You are on the far side of the chasm.  A ne path leads away from the chasm\n" "on this side.",
/*  Room 123  */
	"You're in a long east/west corridor.  A faint rumbling noise can be heard in\n" "the distance.",
/*  Room 124  */
	"The path forks here.  The left fork leads northeast.  A dull rumbling seems\n" "to get louder in that direction.  The right fork leads southeast down a\n" "gentle slope.  The main corridor enters from the west.",
/*  Room 125  */
	"The walls are quite warm here.  From the north can be heard a steady roar,\n" "so loud that the entire cave seems to be trembling.  Another passage leads\n" "south, and a low crawl goes east.",
/*  Room 126  */
	"@145,146,147",
/*  Room 127  */
	"You are in a small chamber filled with large boulders.  The walls are very\n" "warm, causing the air in the room to be almost stifling from the heat.  The\n" "only exit is a crawl heading west, through which is coming a low rumbling.",
/*  Room 128  */
	"You are walking along a gently sloping north/south passage lined with oddly\n" "shaped limestone formations.",
/*  Room 129  */
	"You are standing at the entrance to a large, barren room.  A sign posted\n" "above the entrance reads:\n" "               \"Caution!  Bear in room!\""
/*  Room 130  */
	    "You are inside a barren room.  The center of the room is completely empty\n" "except for some dust.  Marks in the dust lead away toward the far end of the\n" "room.  The only exit is the way you came in.",
/*  Room 131  */
	"You are in a maze of twisting little passages, all different.",
/*  Room 132  */
	"You are in a little maze of twisty passages, all different.",
/*  Room 133  */
	"You are in a twisting maze of little passages, all different.",
/*  Room 134  */
	"You are in a twisting little maze of passages, all different.",
/*  Room 135  */
	"You are in a twisty little maze of passages, all different.",
/*  Room 136  */
	"You are in a twisty maze of little passages, all different.",
/*  Room 137  */
	"You are in a little twisty maze of passages, all different.",
/*  Room 138  */
	"You are in a maze of little twisting passages, all different.",
/*  Room 139  */
	"You are in a maze of little twisty passages, all different.",
/*  Room 140  */
	"@46",
/*  Extra 141  */
	"You are at the northeast end of an immense room, even larger than the giant\n"
	    "room.  It appears to be a repository for the \"adventure\" program.  Massive\n"
	    "torches far overhead bathe the room with smoky yellow light.  Scattered\n"
	    "about you can be seen a pile of bottles (all of them empty), a nursery of\n" "young beanstalks murmuring quietly, a bed of oysters, a bundle of black rods\n" "with rusty stars on their ends, and a collection of brass lanterns.  Off to",
/*  Extra 142  */
	"one side a great many Dwarves are sleeping on the floor, snoring loudly.  A\n"
	    "sign nearby reads:\n"
	    "\n" "                  \"Do NOT disturb the Dwarves!\"\n" "\n" "An immense mirror is hanging against one wall, and stretches to the other\n" "end of the room, where various other sundry objects can be glimpsed dimly in\n" "the distance.",
/*  Extra 143  */
	"You are at the southwest end of the repository.  To one side is a pit full\n"
	    "of fierce green snakes.  On the other side is a row of small wicker cages,\n"
	    "each of which contains a little sulking bird.  In one corner is a bundle of\n" "black rods with rusty marks on their ends.  A large number of velvet pillows\n" "are scattered about on the floor.  A vast mirror stretches off to the",
/*  Extra 144  */
	"northeast.  At your feet is a large steel grate, next to which is a sign\n" "which reads:\n" "                \"Treasure vault.  Keys in main office.\"",
/*  Extra 145  */
	"You are on the edge of a breath-taking view.  Far below you is an active\n"
	    "volcano, from which great gouts of molten lava come surging out, cascading\n"
	    "back down into the depths.  The glowing rock fills the farthest reaches of\n"
	    "the cavern with a blood-red glare, giving everything an eerie, macabre\n" "appearance.  The air is filled with flickering sparks of ash and a heavy\n" "smell of brimstone.  The walls are hot to the touch, and the thundering of",
/*  Extra  146	*/
	"the volcano drowns out all other sounds.  Embedded in the jagged roof far\n"
	    "overhead are myriad formations composed of pure white alabaster, which\n"
	    "scatter their murky light into sinister apparitions upon the walls.  To one\n"
	    "side is a deep gorge, filled with a bizarre chaos of tortured rock which\n" "seems to have been crafted by the Devil Himself.  An immense river of fire\n" "crashes out from the depths of the volcano, burns its way through the gorge,",
/*  Extra  147	*/
	"and plummets into a bottomless pit far off to your left.  To the right, an\n"
	    "immense geyser of blistering steam erupts continuously from a barren island\n"
	    "in the center of a sulfurous lake, which bubbles ominously. The far right\n"
	    "wall is aflame with an incandescence of its own, which lends an additional\n" "infernal splendor to the already hellish scene.  A dark, foreboding passage\n" "exits to the south.",
};

char *pShortRmDesc[] = {
/*  Room 1  */
	"You're at end of road again.",
/*  Room 2  */
	"You're at hill in road.",
/*  Room 3  */
	"You're inside building.",
/*  Room 4  */
	"You're in valley.",
/*  Room 5  */
	"You're in forest.",
/*  Room 6  */
	"You're in forest.",
/*  Room 7  */
	"You're at slit in streambed.",
/*  Room 8  */
	"You're outside grate.",
/*  Room 9  */
	"You're below the grate.",
/*  Room 10  */
	"You're in cobble crawl.",
/*  Room 11  */
	"You're in debris room.",
/*  Room 12  */
	"You are in an awkward sloping east/west canyon.",
/*  Room 13  */
	"You're in bird chamber.",
/*  Room 14  */
	"You're at top of small pit.",
/*  Room 15  */
	"You're in hall of mists.",
/*  Room 16  */
	"The crack is far too small for you to follow.",
/*  Room 17  */
	"You're on east bank of fissure.",
/*  Room 18  */
	"You're in nugget of gold room.",
/*  Room 19  */
	"You're in hall of mt. king.",
/*  Room 20  */
	"You are the the bottom of the pit with a broken neck.",
/*  Room 21  */
	"You didn't make it.",
/*  Room 22  */
	"The dome is unclimbable.",
/*  Room 23  */
	"You're at west end of twopit room.",
/*  Room 24  */
	"You're in east pit.",
/*  Room 25  */
	"You're in west pit.",
/*  Room 26  */
	"You clamber up the plant and scurry through the hole at the top.",
/*  Room 27  */
	"You are on the west side of the fissure in the hall of mists.",
/*  Room 28  */
	"You are in a low N/S passage at a hole in the floor.  The hole goes down to\n" "an E/W passage.",
/*  Room 29  */
	"You are in the south side chamber.",
/*  Room 30  */
	"You are in the west side chamber of the hall of the mountain king.  A\n" "passage continues west and up here.",
/*  Room 31  */
	">$<",
/*  Room 32  */
	"You can't get by the snake.",
/*  Room 33  */
	"You're at \"Y2\".",
/*  Room 34  */
	"You are in a jumble of rock, with cracks everywhere.",
/*  Room 35  */
	"You're at window on pit.",
/*  Room 36  */
	"You're in dirty passage.",
/*  Room 37  */
	"You are on the brink of a small clean climbable pit.  A crawl leads west.",
/*  Room 38  */
	"You are in the bottom of a small pit with a little stream, which enters and\n" "exits through tiny slits.",
/*  Room 39  */
	"You're in dusty rock room.",
/*  Room 40  */
	"You have crawled through a very low wide passage parallel to and north of\n" "the hall of mists.",
/*  Room 41  */
	"You're at west end of hall of mists.",
/*  Room 42  */
	"You are in a maze of twisty little passages, all alike.",
/*  Room 43  */
	"@42",
/*  Room 44  */
	"@42",
/*  Room 45  */
	"@42",
/*  Room 46  */
	"Dead end.",
/*  Room 47  */
	"@46",
/*  Room 48  */
	"@46",
/*  Room 49  */
	"@42",
/*  Room 50  */
	"@42",
/*  Room 51  */
	"@42",
/*  Room 52  */
	"@42",
/*  Room 53  */
	"@42",
/*  Room 54  */
	"@46",
/*  Room 55  */
	"@42",
/*  Room 56  */
	"@46",
/*  Room 57  */
	"You're at brink of pit.",
/*  Room 58  */
	"@46",
/*  Room 59  */
	"You have crawled through a very low wide passage paralled to and north of\n" "the hall of mists.",
/*  Room 60  */
	"You're at east end of long hall.",
/*  Room 61  */
	"You're at west end of long hall.",
/*  Room 62  */
	"You are at a crossover of a high N/S passage and a low E/W one.",
/*  Room 63  */
	"@46",
/*  Room 64  */
	"You're at complex junction.",
/*  Room 65  */
	"You are in bedquilt, a long east/west passage with holes everywhere.  To\n" "explore at random select north, south, up or down.",
/*  Room 66  */
	"You're in swiss cheese room.",
/*  Room 67  */
	"You're at east end of twopit room.",
/*  Room 68  */
	"You're in slab room.",
/*  Room 69  */
	"You are in a secret N/S canyon above a large room.",
/*  Room 70  */
	"You are in a secret N/S canyon above a sizable passage.",
/*  Room 71  */
	"You're at junction of three secret canyons.",
/*  Room 72  */
	"You are in a large low room.  Crawls lead north, se, and sw.",
/*  Room 73  */
	"Dead end crawl.",
/*  Room 74  */
	"You're at secret E/W canyon above tight canyon.",
/*  Room 75  */
	"You are at a wide place in a very tight N/S canyon.",
/*  Room 76  */
	"The canyon here becomes too tight to go further south.",
/*  Room 77  */
	"You are in a tall E/W canyon.  A low tight crawl goes 3 feet north and seems\n" "to open up.",
/*  Room 78  */
	"The canyon runs into a mass of boulders -- dead end.",
/*  Room 79  */
	"The stream flows out through a pair of 1 foot diameter sewer pipes.  It\n" "would be advisable to use the exit.",
/*  Room 80  */
	"@42",
/*  Room 81  */
	"@46",
/*  Room 82  */
	"@46",
/*  Room 83  */
	"@42",
/*  Room 84  */
	"@42",
/*  Room 85  */
	"@46",
/*  Room 86  */
	"@46",
/*  Room 87  */
	"@42",
/*  Room 88  */
	"You're in narrow corridor.",
/*  Room 89  */
	"There is nothing here to climb.  Use \"up\" or \"out\" to leave the pit.",
/*  Room 90  */
	"You have climbed up the plant and out of the pit.",
/*  Room 91  */
	"You're at steep incline above large room.",
/*  Room 92  */
	"You're in giant room.",
/*  Room 93  */
	"The passage here is blocked by a recent cave-in.",
/*  Room 94  */
	"You are at one end of an immense north/south passage.",
/*  Room 95  */
	"You're in cavern with waterfall.",
/*  Room 96  */
	"You're in soft room.",
/*  Room 97  */
	"You're in oriental room.",
/*  Room 98  */
	"You're in misty cavern.",
/*  Room 99  */
	"You're in alcove.",
/*  Room 100  */
	"You're in plover room.",
/*  Room 101  */
	"You're in dark-room.",
/*  Room 102  */
	"You're in arched hall.",
/*  Room 103  */
	"You're in shell room.",
/*  Room 104  */
	"You are in a long sloping corridor with ragged sharp walls.",
/*  Room 105  */
	"You are in a cul-de-sac about eight feet across.",
/*  Room 106  */
	"You're in anteroom.",
/*  Room 107  */
	"You are in a maze of twisty little passages, all different.",
/*  Room 108  */
	"You're at Witt's end.",
/*  Room 109  */
	"You're in mirror canyon.",
/*  Room 110  */
	"You're at window on pit.",
/*  Room 111  */
	"You're at top of stalactite.",
/*  Room 112  */
	"You are in a little maze of twisting passages, all different.",
/*  Room 113  */
	"You're at reservoir.",
/*  Room 114  */
	"@46",
/*  Room 115  */
	"You're at ne end of repository.",
/*  Room 116  */
	"You're at sw end of repository.",
/*  Room 117  */
	"You're on sw side of chasm.",
/*  Room 118  */
	"You're in sloping corridor.",
/*  Room 119  */
	"You are in a secret canyon which exits to the north and east.",
/*  Room 120  */
	"@119",
/*  Room 121  */
	"@119",
/*  Room 122  */
	"You're on ne side of chasm.",
/*  Room 123  */
	"You're in corridor.",
/*  Room 124  */
	"You're at fork in path.",
/*  Room 125  */
	"You're at junction with warm walls.",
/*  Room 126  */
	"You're at breath-taking view.",
/*  Room 127  */
	"You're in chamber of boulders.",
/*  Room 128  */
	"You're in limestone passage.",
/*  Room 129  */
	"You're in front of barren room.",
/*  Room 130  */
	"You're in barren room.",
/*  Room 131  */
	"You are in a maze of twisting little passages, all different.",
/*  Room 132  */
	"You are in a little maze of twisty passages, all different.",
/*  Room 133  */
	"You are in a twisting maze of little passages, all different.",
/*  Room 134  */
	"You are in a twisting little maze of passages, all different.",
/*  Room 135  */
	"You are in a twisty little maze of passages, all different.",
/*  Room 136  */
	"You are in a twisty maze of little passages, all different.",
/*  Room 137  */
	"You are in a little twisty maze of passages, all different.",
/*  Room 138  */
	"You are in a maze of little twisting passages, all different.",
/*  Room 139  */
	"You are in a maze of little twisty passages, all different.",
/*  Room 140  */
	"@46",
};

char *pTextMsg[] = {
/*  Msg 1  */
	"@202,203,204",
/*  Msg 2  */
	"A little dwarf with a big knife blocks your way.",
/*  Msg 3  */
	"A little dwarf just walked around a corner, saw you, threw a little axe at\n" "you which missed, cursed, and ran away.",
/*  Msg 4  */
	"There is a threatening little dwarf in the room with you!",
/*  Msg 5  */
	"One sharp, nasty knife is thrown at you!",
/*  Msg 6  */
	"None of them hit you!",
/*  Msg 7  */
	"One of them gets you!",
/*  Msg 8  */
	"A hollow voice says \"Plugh\".",
/*  Msg 9  */
	"There is no way to go that direction.",
/*  Msg 10  */
	"I am unsure how you are facing.  Use compass points or nearby objects.",
/*  Msg 11  */
	"I don't know in from out here.  Use compass points or name something in the\n" "general direction you want to go.",
/*  Msg 12  */
	"I don't know how to apply that word here.",
/*  Msg 13  */
	"I don't understand that!",
/*  Msg 14  */
	"I'm game.  Would you care to explain how?",
/*  Msg 15  */
	"Sorry, but I am not allowed to give more detail.  I will repeat the long\n" "description of your location.\n",
/*  Msg 16  */
	"It is now pitch dark.  If you proceed you will likely fall into a pit.",
/*  Msg 17  */
	"If you prefer, simply type W rather than West.",
/*  Msg 18  */
	"Are you trying to catch the bird?",
/*  Msg 19  */
	"The bird is frightened right now and you cannot catch it no matter what you\n" "try.  Perhaps you might try later.",
/*  Msg 20  */
	"Are you trying to somehow deal with the snake?",
/*  Msg 21  */
	"You can't kill the snake, or drive it away, or avoid it, or anything like\n" "that.  There is a way to get by, but you don't have the necessary resources\n" "right now.",
/*  Msg 22  */
	"Do you really want to quit now?",
/*  Msg 23  */
	"You fell into a pit and broke every bone in your body!",
/*  Msg 24  */
	"You are already carrying it!",
/*  Msg 25  */
	"You can't be serious!",
/*  Msg 26  */
	"The bird was unafraid when you entered, but as you approach it becomes\n" "disturbed and you cannot catch it.",
/*  Msg 27  */
	"You can catch the bird, but you cannot carry it.",
/*  Msg 28  */
	"There is nothing here with a lock!",
/*  Msg 29  */
	"You aren't carrying it!",
/*  Msg 30  */
	"The little bird attacks the green snake, and in an astounding flurry drives\n" "the snake away.",
/*  Msg 31  */
	"You have no keys!",
/*  Msg 32  */
	"It has no lock.",
/*  Msg 33  */
	"I don't know how to lock or unlock such a thing.",
/*  Msg 34  */
	"It was already locked.",
/*  Msg 35  */
	"The grate is now locked.",
/*  Msg 36  */
	"The grate is now unlocked.",
/*  Msg 37  */
	"It was already unlocked.",
/*  Msg 38  */
	"You have no source of light.",
/*  Msg 39  */
	"Your lamp is now on.",
/*  Msg 40  */
	"Your lamp is now off.",
/*  Msg 41  */
	"There is no way to get past the bear to unlock the chain, which is probably\n" "just as well.",
/*  Msg 42  */
	"Nothing happens.",
/*  Msg 43  */
	"Where?",
/*  Msg 44  */
	"There is nothing here to attack.",
/*  Msg 45  */
	"The little bird is now dead.  Its body disappears.",
/*  Msg 46  */
	"Attacking the snake both doesn't work and is very dangerous.",
/*  Msg 47  */
	"You killed a little dwarf.",
/*  Msg 48  */
	"You attack a little dwarf, but he dodges out of the way.",
/*  Msg 49  */
	"With what? Your bare hands?",
/*  Msg 50  */
	"Good try, but that is an old worn-out magic word.",
/*  Msg 51  */
	"@205,206,207",
/*  Msg 52  */
	"It misses!",
/*  Msg 53  */
	"It gets you!",
/*  Msg 54  */
	"OK\n",
/*  Msg 55  */
	"You can't unlock the keys.",
/*  Msg 56  */
	"You have crawled around in some little holes and wound up back in the main\n" "passage.",
/*  Msg 57  */
	"I don't know where the cave is, but hereabouts no stream can run on the\n" "surface for very long.  I would try the stream.",
/*  Msg 58  */
	"I need more detailed instructions to do that.",
/*  Msg 59  */
	"I can only tell you what you see as you move about and manipulate things.  I\n" "cannot tell you where remote things are.",
/*  Msg 60  */
	"I don't know that word.",
/*  Msg 61  */
	"What?",
/*  Msg 62  */
	"Are you trying to get into the cave?",
/*  Msg 63  */
	"The grate is very solid and has a hardened steel lock.  You cannot enter\n" "without a key, and there are no keys nearby. I would recommend looking\n" "elsewhere for the keys.",
/*  Msg 64  */
	"The trees of the forest are large hardwood oak and maple, with an occasional\n"
	    "grove of pine or spruce.  There is quite a bit of undergrowth, largely birch\n"
	    "and ash saplings plus nondescript bushes of various sorts.  This time of\n" "year visibility is quite restricted by all the leaves, but travel is quite\n" "easy if you detour around the spruce and berry bushes.",
/*  Msg 65  */
	"Welcome to adventure!!  Would you like instructions?",
/*  Msg 66  */
	"Digging without a shovel is quite impractical.  Even with a shovel progress\n" "is unlikely.",
/*  Msg 67  */
	"Blasting requires dynamite.",
/*  Msg 68  */
	"I'm as confused as you are.",
/*  Msg 69  */
	"Mist is a white vapor, usually water.  Seen from time to time in caverns.\n" "It can be found anywhere but is frequently a sign of a deep pit leading down\n" "to water.",
/*  Msg 70  */
	"Your feet are now wet.",
/*  Msg 71  */
	"I think I just lost my appetite.",
/*  Msg 72  */
	"Thank you.  It was delicious!",
/*  Msg 73  */
	"You have taken a drink from the stream.  The water tastes strongly of\n" "minerals, but is not unpleasant.  It is extremely cold.",
/*  Msg 74  */
	"The bottle of water is now empty.",
/*  Msg 75  */
	"Rubbing the electric lamp is not particularly rewarding. Anyway, nothing\n" "exciting happens.",
/*  Msg 76  */
	"Peculiar.  Nothing unexpected happens.",
/*  Msg 77  */
	"Your bottle is empty and the ground is wet.",
/*  Msg 78  */
	"You can't pour that.",
/*  Msg 79  */
	"Watch it!",
/*  Msg 80  */
	"Which way?",
/*  Msg 81  */
	"Oh dear, you seem to have gotten yourself killed.  I might be able to help\n" "you out, but I've never really done this before.  Do you want me to try to\n" "reincarnate you?",
/*  Msg 82  */
	"All right.  But don't blame me if something goes wr......\n" "                    --- POOF !! ---\n" "You are engulfed in a cloud of orange smoke.  Coughing and gasping, you\n" "emerge from the smoke and find...",
/*  Msg 83  */
	"You clumsy oaf, you've done it again!  I don't know how long I can keep this\n" "up.  Do you want me to try reincarnating you again?",
/*  Msg 84  */
	"Okay, now where did i put my orange smoke? ... > POOF! <\n" "Everything disappears in a dense cloud of orange smoke.",
/*  Msg 85  */
	"Now you've really done it!  I'm out of orange smoke!  You don't expect me to\n" "do a decent reincarnation without any orange smoke, do you?",
/*  Msg 86  */
	"Okay, If you're so smart, do it yourself!  I'm leaving!",
/*  Msg 87  */
	"",
/*  Msg 88  */
	"",
/*  Msg 89  */
	"",
/*  Msg 90  */
	"",
/*  Msg 91  */
	"Sorry, but I no longer seem to remember how it was you got here.",
/*  Msg 92  */
	"You can't carry anything more.  You'll have to drop something first.",
/*  Msg 93  */
	"You can't go through a locked steel grate!",
/*  Msg 94  */
	"I believe what you want is right here with you.",
/*  Msg 95  */
	"You don't fit through a two-inch slit!",
/*  Msg 96  */
	"I respectfully suggest you go across the bridge instead of jumping.",
/*  Msg 97  */
	"There is no way across the fissure.",
/*  Msg 98  */
	"You're not carrying anything.",
/*  Msg 99  */
	"You are currently holding the following:",
/*  Msg 100  */
	"It's not hungry (It's merely pinin' for the Fjords).  Besides you have no\n" "bird seed.",
/*  Msg 101  */
	"The snake has now devoured your bird.",
/*  Msg 102  */
	"There's nothing here it wants to eat (Except perhaps you).",
/*  Msg 103  */
	"You fool, Dwarves eat only coal!  Now you've made him REALLY mad !!",
/*  Msg 104  */
	"You have nothing in which to carry it.",
/*  Msg 105  */
	"Your bottle is already full.",
/*  Msg 106  */
	"There is nothing here with which to fill the bottle.",
/*  Msg 107  */
	"Your bottle is now full of water.",
/*  Msg 108  */
	"Your bottle is now full of oil.",
/*  Msg 109  */
	"You can't fill that.",
/*  Msg 110  */
	"Don't be ridiculous!",
/*  Msg 111  */
	"The door is extremely rusty and refuses to open.",
/*  Msg 112  */
	"The plant indignantly shakes the oil off its leaves and asks: \"Water?\".",
/*  Msg 113  */
	"The hinges are quite thoroughly rusted now and won't budge.",
/*  Msg 114  */
	"The oil has freed up the hinges so that the door will now move, although it\n" "requires some effort.",
/*  Msg 115  */
	"The plant has exceptionally deep roots and cannot be pulled free.",
/*  Msg 116  */
	"The Dwarves' knives vanish as they strike the walls of the cave.",
/*  Msg 117  */
	"Something you're carrying won't fit through the tunnel with you.  You'd best\n" "take inventory and drop something.",
/*  Msg 118  */
	"You can't fit this five-foot clam through that little passage!",
/*  Msg 119  */
	"You can't fit this five foot oyster through that little passage!",
/*  Msg 120  */
	"I advise you to put down the clam before opening it. >STRAIN!<",
/*  Msg 121  */
	"I advise you to put down the oyster before opening it. >WRENCH!<",
/*  Msg 122  */
	"You don't have anything strong enough to open the clam.",
/*  Msg 123  */
	"You don't have anything strong enough to open the oyster.",
/*  Msg 124  */
	"A glistening pearl falls out of the clam and rolls away. Goodness, this must\n" "really be an oyster.  (I never was very good at identifying bivalves.)\n" "Whatever it is, it has now snapped shut again.",
/*  Msg 125  */
	"The oyster creaks open, revealing nothing but oyster inside. It promptly\n" "snaps shut again.",
/*  Msg 126  */
	"You have crawled around in some little holes and found your way blocked by a\n" "recent cave-in.  You are now back in the main passage.",
/*  Msg 127  */
	"There are faint rustling noises from the darkness behind you.",
/*  Msg 128  */
	"Out from the shadows behind you pounces a bearded pirate! \"Har, har\" he\n" "chortles, \"I'll just take all this booty and hide it away with me chest deep\n" "in the maze!\".  He snatches your treasure and vanishes into the gloom.",
/*  Msg 129  */
	"A sepulchral voice reverberating through the cave says: \"Cave closing soon.\n" "All adventurers exit immediately through main office.\"",
/*  Msg 130  */
	"A mysterious recorded voice groans into life and announces: \"This exit is\n" "closed.  Please leave via main office.\"",
/*  Msg 131  */
	"It looks as though you're dead.  Well, seeing as how it's so close to\n" "closing time anyway, I think we'll just call it a day.",
/*  Msg 132  */
	"The sepulchral voice entones, \"The cave is now closed.\"  As the echoes fade,\n" "there is a blinding flash of light (and a small puff of orange smoke). . . .\n" "As your eyes refocus you look around and find...",
/*  Msg 133  */
	"There is a loud explosion, and a twenty-foot hole appears in the far wall,\n"
	    "burying the Dwarves in the rubble.  You march through the hole and find\n" "yourself in the main office, where a cheering band of friendly elves carry\n" "the conquering adventurer off into the sunset.",
/*  Msg 134  */
	"There is a loud explosion, and a twenty-foot hole appears in the far wall,\n" "burying the snakes in the rubble.  A river of molten lava pours in through\n" "the hole, destroying everything in its path, including you!!",
/*  Msg 135  */
	"There is a loud explosion, and you are suddenly splashed across the walls of\n" "the room.",
/*  Msg 136  */
	"The resulting ruckus has awakened the Dwarves.  There are now several\n" "threatening little Dwarves in the room with you! Most of them throw knives\n" "at you!  All of them get you!",
/*  Msg 137  */
	"Oh, leave the poor unhappy bird alone.",
/*  Msg 138  */
	"I daresay whatever you want is around here somewhere.",
/*  Msg 139  */
	"I don't know the word \"stop\".   Use \"quit\" if you want to give up.",
/*  Msg 140  */
	"You can't get there from here.",
/*  Msg 141  */
	"You are being followed by a very large, tame bear.",
/*  Msg 142  */
	"@208,209,210",
/*  Msg 143  */
	"Do you indeed wish to quit now?",
/*  Msg 144  */
	"There is nothing here with which to fill the vase.",
/*  Msg 145  */
	"The sudden change in temperature has delicately shattered the vase.",
/*  Msg 146  */
	"It is beyond your power to do that.",
/*  Msg 147  */
	"I don't know how.",
/*  Msg 148  */
	"It is too far up for you to reach.",
/*  Msg 149  */
	"You killed a little Dwarf.  The body vanished in a cloud of greasy black\n" "smoke.",
/*  Msg 150  */
	"The shell is very strong and impervious to attack.",
/*  Msg 151  */
	"What's the matter, can't you read?  Now you'd best start over.",
/*  Msg 152  */
	"The axe bounces harmlessly off the dragon's thick scales.",
/*  Msg 153  */
	"The dragon looks rather nasty.  You'd best not try to get by.",
/*  Msg 154  */
	"The little bird attacks the green dragon, and in an astounding flurry gets\n" "burnt to a cinder.  The ashes blow away.",
/*  Msg 155  */
	"On what?",
/*  Msg 156  */
	"Okay, from now on I'll only describe a place in full the first time you come\n" "to it.  To get the full description say \"look\".",
/*  Msg 157  */
	"Trolls are close relatives with the rocks and have skin as tough as that of\n" "a rhinoceros.  The troll fends off your blows effortlessly.",
/*  Msg 158  */
	"The troll deftly catches the axe, examines it carefully, and tosses it back,\n" "declaring, \"Good workmanship, but it's not valuable enough.\".",
/*  Msg 159  */
	"The troll catches your treasure and scurries away out of sight.",
/*  Msg 160  */
	"The troll refuses to let you cross.",
/*  Msg 161  */
	"There is no longer any way across the chasm.",
/*  Msg 162  */
	"Just as you reach the other side, the bridge buckles beneath the weight of\n" "the bear, which was still following you around. You scrabble desperately for\n" "support, but as the bridge collapses you stumble back and fall into the\n" "chasm.",
/*  Msg 163  */
	"The bear lumbers toward the troll, who lets out a startled shriek and\n" "scurries away.  The bear soon gives up pursuit and wanders back.",
/*  Msg 164  */
	"The axe misses and lands near the bear where you can't get at it.",
/*  Msg 165  */
	"With what?  Your bare hands?  Agains HIS bear hands??",
/*  Msg 166  */
	"The bear is confused;  he only wants to be your friend.",
/*  Msg 167  */
	"For crying out loud, the poor thing is already dead!",
/*  Msg 168  */
	"The bear eagerly wolfs down your food, after which he seems to calm down\n" "considerably, and even becomes rather friendly.",
/*  Msg 169  */
	"The bear is still chained to the wall.",
/*  Msg 170  */
	"The chain is still locked.",
/*  Msg 171  */
	"The chain is now unlocked.",
/*  Msg 172  */
	"The chain is now locked.",
/*  Msg 173  */
	"There is nothing here to which the chain can be locked.",
/*  Msg 174  */
	"There is nothing here to eat.",
/*  Msg 175  */
	"Do you want the hint?",
/*  Msg 176  */
	"Do you need help getting out of the maze?",
/*  Msg 177  */
	"You can make the passages look less alike by dropping things.",
/*  Msg 178  */
	"Are you trying to explore beyond the plover room?",
/*  Msg 179  */
	"There is a way to explore that region without having to worry about falling\n" "into a pit.  None of the objects available is immediately useful in\n" "discovering the secret.",
/*  Msg 180  */
	"Do you need help getting out of here?",
/*  Msg 181  */
	"Don't go west.",
/*  Msg 182  */
	"Gluttony is not one of the Troll's vices.  Avarice, however, is.",
/*  Msg 183  */
	"Your lamp is getting dim.. You'd best start wrapping this up, unless you can\n" "find some fresh batteries.  I seem to recall there's a vending machine in\n" "the maze.  Bring some coins with you.",
/*  Msg 184  */
	"Your lamp has run out of power.",
/*  Msg 185  */
	"There's not much point in wandering around out here, and you can't explore\n" "the cave without a lamp.  So let's just call it a day.",
/*  Msg 186  */
	"There are faint rustling noises from the darkness behind you. As you turn\n"
	    "toward them, the beam of your lamp falls across a bearded pirate.  He is\n"
	    "carrying a large chest.  \"Shiver me timbers!\"  he cries, \"I've been spotted!\n" "I'd best hide meself off to the maze and hide me chest!\".  With that, he\n" "vanished into the gloom.",
/*  Msg 187  */
	"Your lamp is getting dim.  You'd best go back for those batteries.",
/*  Msg 188  */
	"Your lamp is getting dim.. I'm taking the liberty of replacing the\n" "batteries.",
/*  Msg 189  */
	"Your lamp is getting dim, and you're out of spare batteries. You'd best\n" "start wrapping this up.",
/*  Msg 190  */
	"I'm afraid the magazine is written in Dwarvish.",
/*  Msg 191  */
	"\"This is not the maze where the pirate leaves his treasure chest.\"",
/*  Msg 192  */
	"Hmm, this looks like a clue, which means it'll cost you 10 points to read\n" "it.  Should I go ahead and read it anyway?",
/*  Msg 193  */
	"It says, \"There is something strange about this place, such that one of the\n" "words I've always known now has a new effect.\"",
/*  Msg 194  */
	"It says the same thing it did before.",
/*  Msg 195  */
	"I'm afraid I don't understand.",
/*  Msg 196  */
	"\"Congratulations on bringing light into the dark-room!\"",
/*  Msg 197  */
	"You strike the mirror a resounding blow, whereupon it shatters into a myriad\n" "tiny fragments.",
/*  Msg 198  */
	"You have taken the vase and hurled it delicately to the ground.",
/*  Msg 199  */
	"You prod the nearest Dwarf, who wakes up grumpily, takes one look at you,\n" "curses, and grabs for his axe.",
/*  Msg 200  */
	"Is this acceptable?",
/*  Msg 201  */
	"There's no point in suspending a demonstration game.",
/*  Msg 202  */
	"Somewhere nearby is Colossal Cave, where others have found fortunes in\n"
	    "treasure and gold, though it is rumored that some who enter are never seen\n"
	    "again.  Magic is said to work in the cave.  I will be your eyes and hands.\n"
	    "Direct me with commands of 1 or 2 words.  I should warn you that I look at\n"
	    "only the first five letters of each word, so you'll have to enter\n",
/*  Msg 203  */
	"\"Northeast\" as \"ne\" to distinguish it from \"North\".  (Should you get stuck,\n"
	"type \"help\" for some general hints).\n",
/*  Msg 204  */
	"This program was originally developed by Willie Crowther. Most of the\n"
	"features of the current program were added by Don Woods.  This version,\n"
	"written in BDS 8080 C was adapted by Jay R. Jaeger and was later ported to\n"
	"MSC V5.1 on the IBM/PC by Bob Withers and then to Fuzix by Alan Cox.\n",
/*  Msg 205  */
	"I know of places, actions, and things.  Most of my vocabulary describes\n"
	    "places and is used to move you there.  To move, try words like forest,\n"
	    "building, downstream, enter, east, west, north, south, up or down.  I know\n"
	    "about a few special objects, like a black rod hidden in the cave.  These\n"
	    "objects can be manipulated using some of the action words I know. Usually\n"
	    "you will need to give both the object and action words (In either order),",
/*  Msg 206  */
	"but sometimes I can infer the object from the verb alone.  Some objects also\n"
	    "imply verbs; in particular, \"inventory\" implies \"take inventory\", which\n"
	    "causes me to give you a list of what you're carrying.  The objects have side\n"
	    "effects; for instance, the rod scares the bird.  Usually people having\n"
	    "trouble moving just need to try a few more words.  Usually people trying\n"
	    "unsuccessfully to manipulate an object are attempting something beyond their",
/*  Msg 207  */
	"(or my!) capabilities and should try a completely different tack.  To speed\n"
	    "the game you can sometimes move long distances with a single word.  For\n"
	    "example, \"building\" usually gets you to the building from anywhere above\n"
	    "ground except when lost in the forest.  Also, note that cave passages turn a\n"
	    "lot, and that leaving a room to the north does not guarantee entering the\n"
	    "next from the south.\n"
	    "\n"
	    "Good luck!",
/*  Msg 208  */
	"If you want to end your adventure early, say \"quit\".  To suspend you\n"
	    "adventure such that you can continue later say \"suspend\" (or \"pause\" or\n"
	    "\"save\").  To see how well you're doing, say \"score\".  To get full credit for\n"
	    "a treasure, you must have left it safely in the building, though you get\n"
	    "partial credit just for locating it. You lose points for getting killed, or\n"
	    "for quitting, though the former costs you more.  There are also points based",
/*  Msg 209  */
	"on how much (If any) of the cave you've managed to explore;  in particular,\n"
	    "there is a large bonus just for getting in (to distinguish the beginners\n"
	    "from the rest of the pack), and there are other ways to determine whether\n"
	    "you've been through some of the more harrowing sections. If you think you've\n"
	    "found all the treasures, just keep exploring for a while.  If nothing\n"
	    "interesting happens, you haven't found them all yet.  If something",
/*  Msg 210  */
	"interesting DOES happen, it means you're getting a bonus and have an\n"
	    "opportunity to garner many more points in the master's section.  I may\n"
	    "occasionally offer hints in you seem to be having trouble.  If I do, I'll\n"
	    "warn you in advance how much it will affect your score to accept the hints.\n"
	    "Finally, to save paper, you may specify \"brief\", which tells me never to\n"
	    "repeat the full description of a place unless you explicitly ask me to.",
};

struct gameheader {
	uint16_t msg[211];
	uint16_t lshort[141];
	uint16_t odesc[65];
	uint16_t loclong[148];
};

#define out 1

static int crossendian;

static uint16_t endianize(uint16_t v)
{
	if (crossendian)
		v = ((v & 0xFF) << 8) | (v >> 8);
	return v;
}

int main(int argc, char *argv[])
{
	int i = 0;
	long base = 0;
	uint16_t dp;
	int len;
	struct trav t;
	struct gameheader game;

	if (argc == 2 && strcmp(argv[1], "-x") == 0) {
		crossendian = 1;
		argc--;
	}
	if (argc != 1) {
		fprintf(stderr, "%s [-x] >advent.db", argv[0]);
		exit(1);
	}
	base = sizeof(game);
	write(out, &game, sizeof(game));
	dp = sizeof(game);
	while (i < 210) {
		len = strlen(pTextMsg[i]) + 1;
		game.msg[i] = endianize(dp);
		dp += len;
		write(out, pTextMsg[i], len);
		i++;
	}
	game.msg[i] = dp;
	for (i = 0; i < 140; i++) {

		/* 0 terminate as entries don't give the true length */
		len = strlen(pShortRmDesc[i]) + 1;
		game.lshort[i] = endianize(dp);
		write(out, pShortRmDesc[i], len);
		dp += len;
		if (TravTab[i].sTrav > 16) {
			fprintf(stderr, "Trav too big %d = %d\n", i, TravTab[i].sTrav);
			exit(1);
		}
		dp += 16 * sizeof(struct trav) + 1;
		write(out, &TravTab[i].sTrav, 1);
		write(out, TravTab[i].pTrav, 16 * sizeof(struct trav));
	} game.lshort[i] = dp;
	for (i = 0; i < 64; i++) {
		len = strlen(pObjDesc[i]);
		game.odesc[i] = endianize(dp);
		write(out, pObjDesc[i], len);
		dp += len;
	}
	game.odesc[i] = dp;
	for (i = 0; i < 64; i++) {
		len = strlen(pLongRmDesc[i]);
		game.loclong[i] = endianize(dp);
		write(out, pLongRmDesc[i], len);
		dp += len;
	}
	game.loclong[i] = endianize(dp);
	if (lseek(out, 0L, SEEK_SET) == -1) {
		perror("seek");
		exit(1);
	}
	write(out, &game, sizeof(game));
	close(out);
	exit(0);
}
