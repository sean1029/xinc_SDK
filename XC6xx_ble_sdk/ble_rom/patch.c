


void _Ctune_Set(void)
{
	*((unsigned int volatile*)0x40002430) = (unsigned int)(0x3f3f820);	//ctune  DEBUG !!!!!!!!!
}

void HWradio_Program_Dummy_Rx(unsigned char io_Channel)
{
	
	
}
// ((void (*)(void))PATCH_FUN[HWRADIO_TXCOMPLETE])();
void HWradio_TxComplete(void)
{
}

