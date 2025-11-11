#pragma once

#define STOP while(1){__asm__("BKPT");}
#define BREAK __asm__("BKPT")