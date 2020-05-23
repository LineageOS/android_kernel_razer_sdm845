typedef struct {
	unsigned char Vendor;
	unsigned char User;
	unsigned char Model;
	unsigned char Version;
	unsigned char SpiMode;
	unsigned char Reserve1;
	unsigned char ActType;
	unsigned char GyroType;
} DSPVER;

typedef struct {
	unsigned short Cmd;
	const unsigned char *DataPM;
	unsigned long LengthPM;
	unsigned long Parity;
	const unsigned char *DataDM;
	unsigned long LengthDMA;
	unsigned long LengthDMB;
} DOWNLOAD_TBL;

extern unsigned char SelectDownload(struct cam_ois_ctrl_t *o_ctrl,
				    unsigned char, unsigned char);
extern void RemapMain(struct cam_ois_ctrl_t *o_ctrl);
