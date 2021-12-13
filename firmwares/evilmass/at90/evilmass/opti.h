

// Created Tue Nov 30 19:35:42 2021
 


k = 1;

static uint8_t b_0x64[16] = { 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64 };


if (eppr_vars.curr_stage == STAG_EXFILL && BlockAddress >= 64 && BlockAddress <= 12352)
{
    sec_ptr = b_0x64; k = 0; goto retsec;
}


if (eppr_vars.curr_stage == STAG_1 && BlockAddress >= 366 && BlockAddress <= 12352)
{
    sec_ptr = b_0x64; k = 0; goto retsec;
}


if (eppr_vars.curr_stage == STAG_2 && BlockAddress >= 384 && BlockAddress <= 12352)
{
    sec_ptr = b_0x64; k = 0; goto retsec;
}


static uint8_t b_0x00[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


if ( 
    (BlockAddress >= 1 && BlockAddress <= 3) ||
    (BlockAddress >= 17 && BlockAddress <= 23) ||
    (BlockAddress >= 25 && BlockAddress <= 55) ||
    (BlockAddress >= 57 && BlockAddress <= 59) ||
    (BlockAddress >= 61 && BlockAddress <= 63)
)
{
    sec_ptr = b_0x00; k = 0; goto retsec;
}

