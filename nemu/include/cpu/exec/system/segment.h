#ifndef __SEGMENT_H__
#define __SEGMENT_H__

make_helper(lgdt);
make_helper(lidt);
make_helper(mov_rm2sreg);
make_helper(mov_sreg2rm);
make_helper(ljmp);

#endif
