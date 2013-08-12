#pragma once

#define ATAG_NONE	0x00000000

struct TagHeader {
	unsigned int nSize;
	unsigned int ulTag;
};

#define ATAG_CORE	0x54410001

struct TagCore {
	unsigned int ulFlags;	
	unsigned int nPageSize;
	unsigned int ulRootDev;
};

#define ATAG_MEM	0x54410002

struct TagMem32 {
	unsigned int	nSize;
	unsigned int	ulStart;
};


#define ATAG_CMDLINE	0x54410009

struct TagCmdline {
	char	cCmdLine[1];
};

struct Atag {
	struct TagHeader stHdr;

	union {
		struct TagCore		stCore;
		struct TagMem32		stMem;
		struct TagCmdline	stCmdLine;
	}u;
};


#define TAG_NEXT(t)	((struct Atag *)((unsigned int *)(t) + (t)->stHdr.nSize))
#define TAG_SIZE(type)	((sizeof(struct TagHeader) + sizeof(struct type)) >> 2)

#if 1
#define FOR_EACH_TAG(t, base) \
	for (t = base; t->stHdr.nSize; t = TAG_NEXT(t))
#endif


#define CONF_RAM_BANK_NUM 		1
#define DEFAULT_KCMDLINE_LEN		1024

