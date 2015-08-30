/* Linker script for the bootloader */
_ebase_address = 0x9FC01000;

/*************************************************************************
 * Memory Address Equates
 * _RESET_ADDR                    -- Reset Vector
 * _BEV_EXCPT_ADDR                -- Boot exception Vector
 * _DBG_EXCPT_ADDR                -- In-circuit Debugging Exception Vector
 *************************************************************************/
_RESET_ADDR                    = 0xBFC00000;
_BEV_EXCPT_ADDR                = 0xBFC00380;
_DBG_EXCPT_ADDR                = 0xBFC00480;

/*************************************************************************
 * Memory Regions
 *************************************************************************/
MEMORY
{
  kseg0_program_mem     (rx)  : ORIGIN = 0x9D000000, LENGTH = 0x6000
  kseg0_boot_mem              : ORIGIN = 0x9FC00490, LENGTH = 0x970
  exception_mem               : ORIGIN = 0x9FC01000, LENGTH = 0x1000
  kseg1_boot_mem              : ORIGIN = 0xBFC00000, LENGTH = 0x490
}

INCLUDE common_mx695F512H.ld
