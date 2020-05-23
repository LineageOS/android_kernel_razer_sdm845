#include <cam_sensor_cmn_header.h>
#include "cam_debug_util.h"
#include "cam_ois_dev.h"
#include "PhoneDownload.h"
#include "LC898124EP3_Code_3_1_1.h"

#define BURST_LENGTH_PM (12 * 5)
#define BURST_LENGTH_DM (10 * 6)
#define BURST_LENGTH BURST_LENGTH_PM

void CntWrt(struct cam_ois_ctrl_t *o_ctrl, unsigned char *data,
	    unsigned short size)
{
	struct cam_sensor_i2c_reg_setting i2c_reg_setting;
	int32_t rc = 0, cnt = 0;
	uint16_t total_bytes = 0, data_cnt = 0, r_addr = 0;
	uint8_t *ptr = NULL;

	CAM_DBG(CAM_OIS, "[LC898124] E size: %d data[2]: %d i2c_freq_mode: %d",
		size, data[2],
		o_ctrl->io_master_info.cci_client->i2c_freq_mode);

	total_bytes = size - 2;
	i2c_reg_setting.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_setting.size = total_bytes;
	i2c_reg_setting.delay = 0;
	CAM_DBG(CAM_OIS, "[LC898124] i2c_reg_setting.size: %d ",
		i2c_reg_setting.size);
	i2c_reg_setting.reg_setting =
		(struct cam_sensor_i2c_reg_array *)kzalloc(
			sizeof(struct cam_sensor_i2c_reg_array) * total_bytes,
			GFP_KERNEL);
	if (!i2c_reg_setting.reg_setting) {
		CAM_ERR(CAM_OIS, "Failed in allocating i2c_array");
	}

	CAM_DBG(CAM_OIS, "[LC898124] cnt: %d reg_addr: 0x%x", cnt,
		i2c_reg_setting.reg_setting[cnt].reg_addr);
	o_ctrl->io_master_info.cci_client->i2c_freq_mode = 1;
	ptr = (uint8_t *)data;
	r_addr = data[data_cnt] << 8;
	data_cnt++;
	r_addr |= data[data_cnt];
	ptr += 2;
	for (cnt = 0; cnt < total_bytes; cnt++, ptr++) {
		i2c_reg_setting.reg_setting[cnt].reg_addr = r_addr;
		i2c_reg_setting.reg_setting[cnt].reg_data = *ptr;
		i2c_reg_setting.reg_setting[cnt].delay = 0;
		i2c_reg_setting.reg_setting[cnt].data_mask = 0;
	}

	rc = camera_io_dev_write_continuous(&(o_ctrl->io_master_info),
					    &i2c_reg_setting, 1);
	if (rc < 0) {
		CAM_ERR(CAM_OIS, "[LC898124] OIS seq write failed %d", rc);
	}
	kfree(i2c_reg_setting.reg_setting);
	CAM_DBG(CAM_OIS, "[LC898124] X");
}

int RamRead32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, void *data)
{
	int rc = 0;
	CAM_DBG(CAM_OIS, "[LC898124] E");

	rc = camera_io_dev_read(&(o_ctrl->io_master_info), addr, data,
				CAMERA_SENSOR_I2C_TYPE_WORD,
				CAMERA_SENSOR_I2C_TYPE_DWORD);
	CAM_DBG(CAM_OIS, "0x%x = 0x%x", addr, *(uint32_t *)data);
	CAM_DBG(CAM_OIS, "[LC898124] X");
	return rc;
}

int RamWrite32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t data)
{
	int rc = 0;
	struct cam_sensor_i2c_reg_setting write_setting;
	struct cam_sensor_i2c_reg_array i2c_reg_array;
	CAM_DBG(CAM_OIS, "[LC898124] E");

	i2c_reg_array.reg_addr = addr;
	i2c_reg_array.reg_data = data;
	i2c_reg_array.delay = 1;
	write_setting.reg_setting = &i2c_reg_array;
	write_setting.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	write_setting.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
	write_setting.size = 1;
	write_setting.delay = 0;

	rc = camera_io_dev_write(&(o_ctrl->io_master_info), &write_setting);
	CAM_DBG(CAM_OIS, "sid: 0x%xaddr:0x%x data:0x%x rc:%d",
		o_ctrl->io_master_info.cci_client->sid, addr, data, rc);
	CAM_DBG(CAM_OIS, "[LC898124] X");
	return rc;
}

void DMIOWrite32(struct cam_ois_ctrl_t *o_ctrl, uint32_t IOadrs,
		 uint32_t IOdata)
{
	unsigned char data[10];
	data[0] = 0xC0;
	data[1] = 0x00;
	data[2] = (unsigned char)(IOadrs >> 24);
	data[3] = (unsigned char)(IOadrs >> 16);
	data[4] = (unsigned char)(IOadrs >> 8);
	data[5] = (unsigned char)(IOadrs >> 0);
	data[6] = (unsigned char)(IOdata >> 24);
	data[7] = (unsigned char)(IOdata >> 16);
	data[8] = (unsigned char)(IOdata >> 8);
	data[9] = (unsigned char)(IOdata >> 0);
	CntWrt(o_ctrl, data, 10);
};

unsigned char DownloadToEP3(struct cam_ois_ctrl_t *o_ctrl,
			    const unsigned char *DataPM, uint32_t LengthPM,
			    uint32_t Parity, const unsigned char *DataDM,
			    uint32_t LengthDMA, uint32_t LengthDMB)
{
	uint32_t i, j;
	unsigned char data[64];
	unsigned char Remainder;
	uint32_t UlReadVal, UlCnt;
	uint32_t ReadVerifyPM, ReadVerifyDMA, ReadVerifyDMB;
	CAM_DBG(CAM_OIS, "[LC898124] E");

	RamRead32A(o_ctrl, 0xF00F, &UlReadVal);
	if (UlReadVal == 0x20161121)
		return (3);

	//----------------------------------------------------------------------
	// 0.
	//----------------------------------------------------------------------
	RamWrite32A(o_ctrl, 0xC000, 0xE0500C);
	RamRead32A(o_ctrl, 0xD000, &UlReadVal);
	switch ((unsigned char)UlReadVal) {
	case 0x0A:
		break;
	case 0x01:
		DMIOWrite32(o_ctrl, 0xD000AC, 0x00001000);
		msleep(6);
		break;
	default:
		return (1);
	}
	CAM_DBG(CAM_OIS, "[LC898124] 0");

	//----------------------------------------------------------------------
	// 1.
	//----------------------------------------------------------------------
	data[0] = 0x30;
	data[1] = 0x00;
	data[2] = 0x10;
	data[3] = 0x00;
	data[4] = 0x00;
	CntWrt(o_ctrl, data, 5);

	data[0] = 0x40;
	Remainder = ((LengthPM * 5) / BURST_LENGTH_PM);
	for (i = 0; i < Remainder; i++) {
		UlCnt = 1;
		for (j = 0; j < BURST_LENGTH_PM; j++)
			data[UlCnt++] = *DataPM++;

		CntWrt(o_ctrl, data, BURST_LENGTH_PM + 1);
	}
	Remainder = ((LengthPM * 5) % BURST_LENGTH_PM);
	if (Remainder != 0) {
		UlCnt = 1;
		for (j = 0; j < Remainder; j++)
			data[UlCnt++] = *DataPM++;

		CntWrt(o_ctrl, data, UlCnt);
	}

	data[0] = 0xF0;
	data[1] = 0x0A;
	data[2] = (unsigned char)((LengthPM & 0xFF00) >> 8);
	data[3] = (unsigned char)((LengthPM & 0x00FF) >> 0);
	CntWrt(o_ctrl, data, 4);
	CAM_DBG(CAM_OIS, "[LC898124] 1");

	//----------------------------------------------------------------------
	// 2.
	//----------------------------------------------------------------------
	RamWrite32A(o_ctrl, 0x0100, 0);
	RamWrite32A(o_ctrl, 0x0104, 0);

	Remainder = ((LengthDMA * 6 / 4) / BURST_LENGTH_DM);
	for (i = 0; i < Remainder; i++) {
		CntWrt(o_ctrl, (unsigned char *)DataDM, BURST_LENGTH_DM);
		DataDM += BURST_LENGTH_DM;
	}
	Remainder = ((LengthDMA * 6 / 4) % BURST_LENGTH_DM);
	if (Remainder != 0) {
		CntWrt(o_ctrl, (unsigned char *)DataDM,
				(unsigned char)Remainder);
	}
	DataDM += Remainder;

	Remainder = ((LengthDMB * 6 / 4) / BURST_LENGTH_DM);
	for (i = 0; i < Remainder; i++) {
		CntWrt(o_ctrl, (unsigned char *)DataDM, BURST_LENGTH_DM);
		DataDM += BURST_LENGTH_DM;
	}
	Remainder = ((LengthDMB * 6 / 4) % BURST_LENGTH_DM);
	if (Remainder != 0) {
		CntWrt(o_ctrl, (unsigned char *)DataDM,
				(unsigned char)Remainder);
	}
	CAM_DBG(CAM_OIS, "[LC898124] 2");

	//----------------------------------------------------------------------
	// 3.
	//----------------------------------------------------------------------
	RamRead32A(o_ctrl, 0x0110, &ReadVerifyPM);
	RamRead32A(o_ctrl, 0x0100, &ReadVerifyDMA);
	RamRead32A(o_ctrl, 0x0104, &ReadVerifyDMB);

	if ((ReadVerifyPM + ReadVerifyDMA + ReadVerifyDMB) != Parity) {
		return (2);
	}
	CAM_DBG(CAM_OIS, "[LC898124] X");
	return (0);
}

void RemapMain(struct cam_ois_ctrl_t *o_ctrl)
{
	RamWrite32A(o_ctrl, 0xF000, 0x00000000);
}

unsigned char GetInfomationBeforeDownlaod(DSPVER *Info,
				   const unsigned char *DataDM,
				   uint32_t LengthDM)
{
	uint32_t i;
	Info->ActType = 0;
	Info->GyroType = 0;
	CAM_DBG(CAM_OIS, "[LC898124] E LengthDM:%d", LengthDM);

	for (i = 0; i < LengthDM; i += 6) {
		if ((DataDM[0 + i] == 0xA0) && (DataDM[1 + i] == 0x00)) {
			Info->Vendor = DataDM[2 + i];
			Info->User = DataDM[3 + i];
			Info->Model = DataDM[4 + i];
			Info->Version = DataDM[5 + i];
			if ((DataDM[6 + i] == 0xA0) &&
			    (DataDM[7 + i] == 0x04)) {
				Info->SpiMode = DataDM[8 + i];
				Info->ActType = DataDM[10 + i];
				Info->GyroType = DataDM[11 + i];
			}
			return (0);
		}
	}
	CAM_DBG(CAM_OIS, "[LC898124] X");
	return (1);
}

const DOWNLOAD_TBL DTbl[] = {
	{ 0x0101, LC898124EP3_PM_3_1_1, LC898124EP3_PMSize_3_1_1,
	  (uint32_t)(LC898124EP3_PMCheckSum_3_1_1 +
		    LC898124EP3_DMA_CheckSum_3_1_1 +
		    LC898124EP3_DMB_CheckSum_3_1_1),
	  LC898124EP3_DM_3_1_1, LC898124EP3_DMA_ByteSize_3_1_1,
	  LC898124EP3_DMB_ByteSize_3_1_1 },
	{ 0xFFFF, (void *)0, 0, 0, (void *)0, 0, 0 }
};

unsigned char SelectDownload(struct cam_ois_ctrl_t *o_ctrl, unsigned char Model,
			     unsigned char ActSelect)
{
	DSPVER Dspcode;
	DOWNLOAD_TBL *ptr;
	CAM_ERR(CAM_OIS, "[LC898124] E");

	ptr = (DOWNLOAD_TBL *)DTbl;
	while (ptr->Cmd != 0xFFFF) {
		if (ptr->Cmd == (((unsigned short)Model << 8) + ActSelect))
			break;
		ptr++;
	}
	if (ptr->Cmd == 0xFFFF)
		return (0xF0);

	if (GetInfomationBeforeDownlaod(&Dspcode, ptr->DataDM,
			(ptr->LengthDMA + ptr->LengthDMB)) != 0) {
		CAM_ERR(CAM_OIS, "[LC898124] X");
		return (0xF1);
	}

	if ((ActSelect != Dspcode.ActType) || (Model != Dspcode.Model)) {
		CAM_ERR(CAM_OIS, "[LC898124] ActSelect: %d %d Model: %d %d",
			ActSelect, Dspcode.ActType, Model, Dspcode.Model);
		return (0xF2);
	}

	return (DownloadToEP3(o_ctrl, ptr->DataPM, ptr->LengthPM, ptr->Parity,
			      ptr->DataDM, ptr->LengthDMA, ptr->LengthDMB));
}
