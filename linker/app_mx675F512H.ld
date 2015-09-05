/* Linker script for the main application */
_ebase_address = 0x9D007000;

/*************************************************************************
 * Memory Address Equates
 * _RESET_ADDR                    -- Reset Vector
 * _BEV_EXCPT_ADDR                -- Boot exception Vector
 * _DBG_EXCPT_ADDR                -- In-circuit Debugging Exception Vector
 *************************************************************************/
_RESET_ADDR                    = 0x9D008000;
_BEV_EXCPT_ADDR                = 0x9D008380;
_DBG_EXCPT_ADDR                = 0x9D008480;


/*************************************************************************
 * Memory Regions
 *************************************************************************/
MEMORY
{
  kseg0_program_mem     (rx)  : ORIGIN = 0x9D008490, LENGTH = 0x81000 - 0x8490
  kseg0_boot_mem              : ORIGIN = 0x9D000000, LENGTH = 0x0
  exception_mem               : ORIGIN = 0x9D007000, LENGTH = 0x1000
  kseg1_boot_mem              : ORIGIN = 0x9D008000, LENGTH = 0x490
}

INCLUDE common_mx695F512L.ld
