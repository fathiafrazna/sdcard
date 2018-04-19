// Afrendy Bayu
// Depok, 27 Sept 2013
// 


#include "FreeRTOS.h"
#include "task.h"
#include "ff12b/fatfs/ff.h"
#include "monita.h"
#include "ap_file.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "hardware.h"
#include "semphr.h"
#include "ff12b/fatfs/shell_fs.h"
//#include "/modul/ff/fatfs/sysdefs.h"

//#define debug

//#define debug2

#ifdef PAKAI_FILE_SIMPAN

char* strcmd_conf[]  = {"env", "kanal", "sumber", "data", "files", "end",NULL};
char* strcmd_env[]   = {"flag","nama", "sn","server","port","file","kirim","mode_kirim","int_data","int_relay","gps_en","id_r1","id_r2","utc", "idslave", "debug1", "debug2", NULL};
char* strcmd_kanal[] = {"flag","kalib", "status", NULL};
char* strcmd_smbr[]  = {"flag","nama", "ip", "alamat", "stack", NULL};
char* strcmd_data[]  = {"flag","no", "nama", "id", "satuan", "status", "batas", "formula", NULL};
char* strcmd_files[] = {"flag","jumlah", "urut", NULL};
//char isi[512] __attribute__ ((section (".usbram1")));
enum xcmd_conf	{
	CONF_ENV, CONF_KANAL, CONF_SUMBER, CONF_DATA, CONF_FILES, CONF_END
} cmd_conf		__attribute__ ((section (".usbram1")));

enum xcmd_conf_env	{
	ENV_FLAG,ENV_NAMA, ENV_SN, ENV_SERVER,ENV_DOMAIN,ENV_DOMAIN_MODE, ENV_PORT,ENV_FILE, ENV_KIRIM, ENV_MODE,ENV_INT_DT, ENV_INT_RL,ENV_GPS_EN, ENV_ID_R1,ENV_ID_R2,ENV_UTC , ENV_IDSLAVE, ENV_DEBUG1, ENV_DEBUG2
} cmd_conf_env		__attribute__ ((section (".usbram1")));

enum xcmd_conf_kanal	{
	KNL_FLAG,KNL_KALIB, KNL_STATUS
} cmd_conf_kanal		__attribute__ ((section (".usbram1")));

enum xcmd_conf_sumber	{
	SMBR_FLAG,SMBR_NAMA, SMBR_IP, SMBR_ALAMAT, SMBR_STACK, SMBR_FORMULA
} cmd_conf_sumber		__attribute__ ((section (".usbram1")));

enum xcmd_conf_data	{
	DATA_FLAG,DATA_NO, DATA_NAMA, DATA_ID, DATA_SATUAN, DATA_STATUS, DATA_BATAS, DARA_FORMULA
} cmd_conf_data		__attribute__ ((section (".usbram1")));

enum xcmd_conf_files	{
	FILES_FLAG,FILES_JUMLAH, FILES_URUT
} cmd_conf_files		__attribute__ ((section (".usbram1")));

struct sconf	{
	int cmd;
	int index;
	char conf[128];
}		__attribute__ ((section (".usbram1")));

struct sconf st_conf __attribute__ ((section (".usbram1")));
#define FORMAT_SIMPAN_STRING

extern struct minmea_sentence_gsv frame_gsv;
extern struct minmea_sentence_rmc frame_rmc;

void parsing_cmd_setting_utama(char *str)	{
	char *pch, *pch2;
	int ni=-1, i;
	int jmlIs = nIstilah(strcmd_conf);

	pch = (char*) strtok (str,"[]");
	for(i=0; i<jmlIs; i++)	{
		pch2 = strstr (str, strcmd_conf[i]);
		if (pch2 != NULL)	{
			//printf("%d, pch2: %s\r\n", i, pch2);
			st_conf.cmd = i;
		}
	}

	//printf ("isi: %d - %s, %d\r\n", st_conf.cmd, pch, jmlIs);
}

int flag_save_env=0;
struct t_env *st_env;
int busy=1;
//extern char buf_lfn[];
extern xSemaphoreHandle xSemSD;
extern char abs_path[128];

int parsing_cfg_env(int i, char *str )	{
	int nx, num , flag_env=0;
	char tstr[32], *pch;
	
	pch = strchr(str, '=') + 1;
	pch = strtok(pch, "\r\n");
	
	if (i==ENV_FLAG)	{
		sscanf(str, "flag = %d", &nx);
		uprintf("   ENV flag:%d\r\n", nx);
		flag_env = nx;
	}
	
	if(flag_env==1 ){  //1: start env , 2: end env, 0 : unknown
		
		if(flag_save_env==1){
			vPortFree(st_env);
		}
		
		st_env = pvPortMalloc( sizeof (struct t_env) );
		
		if (st_env==NULL)	{
			printf("  GAGAL alokmem !");
			vPortFree (st_env);
			return;
		}
		memcpy((char *) st_env, (char *) ALMT_ENV, (sizeof (struct t_env)));
		flag_save_env=1;
	}

	if (i==ENV_NAMA)	{
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   ENV nama:%s\r\n", pch);
		strcpy(st_env->nama_board, pch);
	}
	else if (i==ENV_SN)	{
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   ENV SN  :%s\r\n", pch);
		strcpy(st_env->SN, pch);
	}
	else if (i==ENV_SERVER){
		do	{
			pch++;
		} while (*pch==' ');
		strcpy(tstr, pch);
	
		uint ret_ip = baca_ip( tstr );
		if (ret_ip > 0)	{
			st_env->IP0 = (unsigned char)(ret_ip >> 24);
			st_env->IP1 = (unsigned char)(ret_ip >> 16);
			st_env->IP2 = (unsigned char)(ret_ip >> 8);
			st_env->IP3 = (unsigned char)(ret_ip);
			printf(" IP Server :%d.%d.%d.%d\r\n", st_env->IP0, st_env->IP1, st_env->IP2, st_env->IP3);
		} 
	}
	else if (i==ENV_PORT){
		sscanf(str, "port = %d", &nx);
		uprintf("   ENV port:%d\r\n", nx);
		st_env->Port = nx;
	}
	else if (i==ENV_FILE){
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   ENV File  :%s\r\n", pch);
		strcpy(st_env->berkas, pch);
	}
	else if (i==ENV_DOMAIN){
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   ENV Domain  :%s\r\n", pch);
		strcpy(st_env->domain, pch);
	}
	else if (i==ENV_DOMAIN_MODE){
		sscanf(str, "domain mode = %d", &nx);
		uprintf("   ENV Domain mode  :%d\r\n", nx);
		st_env->domain_mode= nx;
	}
	else if (i==ENV_KIRIM){
		sscanf(str, "kirim = %d", &nx);
		uprintf("   ENV kirim:%d\r\n", nx);
		st_env->statusWebClient = nx;
	}
	else if (i==ENV_MODE){
		sscanf(str, "mode_kirim = %d", &nx);
		uprintf("   ENV mode:%d\r\n", nx);
		st_env->mode_kirim = nx;
	}
	else if (i==ENV_INT_DT){
		sscanf(str, "int_data = %d", &nx);
		uprintf("   ENV int dt:%d\r\n", nx);
		st_env->intReset = nx;
	}
	else if (i==ENV_INT_RL){
		sscanf(str, "int_relay = %d", &nx);
		uprintf("   ENV int rl:%d\r\n", nx);
		st_env->intervalkirim = nx;
	}
	else if (i==ENV_GPS_EN){
		sscanf(str, "gps_en = %d", &nx);
		uprintf("   ENV gps_en:%d\r\n", nx);
		st_env->gps_en = nx;
	}
	else if (i==ENV_ID_R1){
		sscanf(str, "id_r1 = %d", &nx);
		uprintf("   ENV id Relay 1:%d\r\n", nx);
		st_env->idrelay1 = nx;
	}
	else if (i==ENV_ID_R2){
		sscanf(str, "id_r2 = %d", &nx);
		uprintf("   ENV id Relay 2:%d\r\n", nx);
		st_env->idrelay2 = nx;
	}
	else if (i==ENV_UTC){
		sscanf(str, "utc = %d", &nx);
		uprintf("   ENV utc:%d\r\n", nx);
		st_env->UTC = nx;
	}
	else if (i==ENV_IDSLAVE)	{
		sscanf(str, "idslave = %d", &nx);
		uprintf("   ENV idslave:%d\r\n", nx);
		st_env->almtSlave = nx;
	}
	else if (i==ENV_DEBUG1)	{
		sscanf(str, "debug1 = %d", &nx);
		uprintf("   ENV debug1:%d\r\n", nx);
		st_env->prioDebug  = nx;
	}
	else if (i==ENV_DEBUG2)	{
		sscanf(str, "debug2 = %d", &nx);
		uprintf("   ENV debug2:%d\r\n", nx);
		st_env->prioDebug2  = nx;
	}
	
	if(flag_env==2 && flag_save_env==1){
	simpan_st_rom(SEKTOR_ENV, ENV, 0, (unsigned short *) st_env, 0);
	vPortFree (st_env);
	flag_env=0;
	flag_save_env=0;
	}
	
}

int parsing_cfg_kanal(int i, char *str)	{
	char *pch, *pch2;
	int nx, st, flag_env=0;
	float m,c;
	
	//struct t_env *st_env;
	//st_env = pvPortMalloc( sizeof (struct t_env) );
	if (i==KNL_FLAG)	{
		sscanf(str, "flag = %d", &nx);
		uprintf("   ENV flag:%d\r\n", nx);
		flag_env = nx;
	}
	
	if (st_env==NULL)	{
		printf("  GAGAL alokmem !");
		vPortFree (st_env);
		return;
	}
	//memcpy((char *) st_env, (char *) ALMT_ENV, (sizeof (struct t_env)));
	
	if (i==KNL_KALIB)	{
		sscanf(str, "kalib%d = %f %f", &nx, &m, &c);
		uprintf("   KALIB  [%d] m:%.3f, c:%.3f\r\n", nx, m, c);
		st_env->kalib[nx - 1].m = m;
		st_env->kalib[nx - 1].C = c;
	}
	else if (i==KNL_STATUS)	{
		sscanf(str, "status%d = %d", &nx, &st);
		uprintf("   STATUS [%d] st:%d\r\n", nx, st);
		st_env->kalib[nx - 1].status = st;
	}
	
	if(flag_env==2 && flag_save_env==1){
	simpan_st_rom(SEKTOR_ENV, ENV, 0, (unsigned short *) st_env, 0);
	vPortFree (st_env);
	flag_save_env=0;
	}
	
}

int flag_save_sbr=0;
struct t_sumber *st_sumber;


int parsing_cfg_sumber(int i, char *str)	{
	int nx, num, flag_sbr=0;
	char tstr[32], *pch;
	
	pch = strchr(str, '=') + 1;
	pch = strtok(pch, "\r\n");
	
	
	
	if (i==SMBR_FLAG)	{
		sscanf(str, "flag = %d", &nx);
		uprintf("   SBR flag:%d\r\n", nx);
		flag_sbr = nx;
	}
	
	if(flag_sbr==1 ){  //1: start env , 2: end env, 0 : unknown
		
		if(flag_save_sbr==1){
			vPortFree(st_sumber);
		}
		
		st_sumber = pvPortMalloc( JML_SUMBER * sizeof (struct t_sumber) );
		if (st_sumber == NULL)	{
			printf(" %s(): ERR allok memory gagal !\r\n", __FUNCTION__);
			vPortFree(st_sumber);
			return -1;
		}
		memcpy((char *) st_sumber, (char *) ALMT_SUMBER, (JML_SUMBER * sizeof (struct t_sumber)));
		flag_save_sbr=1;
	}
	
	if (i==SMBR_NAMA)	{
		sscanf(str, "nama%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   SUMBER [%d] nama:%s\r\n", nx, pch);
		strcpy(st_sumber[nx].nama,pch);
	}
	else if (i==SMBR_IP)	{
		sscanf(str, "ip%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   SUMBER [%d] ip:%s\r\n", nx, pch);
		//strcpy(st_sumber[nx].IP0,pch);
	}
	else if (i==SMBR_ALAMAT)	{
		sscanf(str, "alamat%d = %d", &nx, &num);
		uprintf("   SUMBER [%d] alamat:%d\r\n", nx, num);
		st_sumber[nx].alamat=num;
	}
	else if (i==SMBR_STACK)		{
		sscanf(str, "stack%d = %d", &nx, &num);
		uprintf("   SUMBER [%d] stack:%d\r\n", nx, num);
		st_sumber[nx].stack=num;		
	}
	else if (i==SMBR_FORMULA)	{
		sscanf(str, "formula%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		uprintf("   SUMBER [%d] formula:%s\r\n", nx, pch);
		strcpy(st_sumber[nx].form,pch);
	}
	
	if(flag_sbr==2 && flag_save_sbr==1){
	simpan_st_rom(SEKTOR_ENV, SUMBER, 1, (unsigned short *) st_sumber, 0);
	vPortFree(st_sumber);
	flag_save_sbr=0;
	} 
}

int flag_save_data=0,flag_cpy=0,lok=0;
struct t_data *st_data;


int parsing_cfg_data(int i, char *str)	{
	int nx, num ,flag_data=0;
	char tstr[32], *pch;
	float rL, aLL, aL, aH, aHH, rH;
	int nox;
	
	pch = strchr(str, '=') + 1;
	pch = strtok(pch, "\r\n");
	
	//struct t_data *st_data;		
	if (i==DATA_FLAG)	{
		sscanf(str, "flag = %d", &nx);
		uprintf("   DATA flag:%d\r\n", nx);
		flag_data = nx;
	}
	
	if(flag_data==1 ){  //1: start env , 2: end env, 0 : unknown
		
		if(flag_save_data==1 || flag_cpy==1){
			vPortFree(st_data);
			flag_cpy=0;
			flag_save_data=0;
			lok=0;
			
		}
		
		st_data = pvPortMalloc( PER_SUMBER * sizeof (struct t_data) );
		if (st_data == NULL)	{
			printf(" %s(): ERR allok memory gagal !\r\n", __FUNCTION__);
			vPortFree (st_data);
			flag_cpy=0;
			flag_save_data=0;
			lok=0;
			return 3;
		}
		
		
		flag_save_data=1;
	}
	
	if(i==DATA_NO){
		sscanf(str, "no = %d", &nx);
		nx--;
		//nox = nx % PER_SUMBER;
		
		if(flag_cpy==0){
			lok = (int) nx;
			memcpy((char *) st_data, (char *) ALMT_DATA+(lok*JML_KOPI_TEMP), (PER_SUMBER * sizeof (struct t_data)));
			flag_cpy=1;
		}
	}
	
	if (i==DATA_NAMA)	{
		sscanf(str, "nama%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		#ifdef debug
		uprintf("   DATA [%d] nama:%s\r\n", nx, pch);
		#endif
		nx--;
		nox = nx % PER_SUMBER;
		//lok = (int) (nx/PER_SUMBER);
		
		strcpy(st_data[nox].nama, pch);
	}
	else if (i==DATA_ID)	{
		sscanf(str, "id%d = %d", &nx, &num);
		#ifdef debug
		uprintf("   DATA [%d] id:%d\r\n", nx, num);
		#endif
		nx--;
		nox = nx % PER_SUMBER;
		//lok = (int) (nx/PER_SUMBER);
		//memcpy((char *) st_data, (char *) ALMT_DATA+(lok*JML_KOPI_TEMP), (PER_SUMBER * sizeof (struct t_data)));
		st_data[nox].id = num;
	}
	else if (i==DATA_SATUAN)	{
		sscanf(str, "satuan%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		#ifdef debug
		uprintf("   DATA [%d] satuan:%s\r\n", nx, pch);
		#endif
		nx--;
		nox = nx % PER_SUMBER;
		//lok = (int) (nx/PER_SUMBER);
		//memcpy((char *) st_data, (char *) ALMT_DATA+(lok*JML_KOPI_TEMP), (PER_SUMBER * sizeof (struct t_data)));
		strcpy(st_data[nox].satuan, pch);
	}
	else if (i==DATA_STATUS)	{
		sscanf(str, "status%d = ", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		num = (strcmp(pch, "MATI"))?0:1;
		#ifdef debug
		uprintf("   DATA [%d] status: [%d] %s\r\n", nx, num, pch);
		#endif
		nx--;
		nox = nx % PER_SUMBER;
		//lok = (int) (nx/PER_SUMBER);
		//memcpy((char *) st_data, (char *) ALMT_DATA+(lok*JML_KOPI_TEMP), (PER_SUMBER * sizeof (struct t_data)));
		st_data[nox].status = num;
	}
	else if (i==DATA_BATAS)	{
		sscanf(str, "batas%d = %f %f %f %f %f %f", &nx, &rL, &aLL, &aL, &aH, &aHH, &rH);
		#ifdef debug
			uprintf("   DATA [%d] rL:%.1f, aLL:%.1f, aL:%.1f, aH:%.1f, aHH:%.1f, rH:%.1f\r\n", \
			nx, rL, aLL, aL, aH, aHH, rH);
		#endif
		nx--;
		nox = nx % PER_SUMBER;
		//lok = (int) (nx/PER_SUMBER);
		//memcpy((char *) st_data, (char *) ALMT_DATA+(lok*JML_KOPI_TEMP), (PER_SUMBER * sizeof (struct t_data)));
		st_data[nox].rangeL = rL;
		st_data[nox].batasLL = aLL;
		st_data[nox].batasL  = aL;
		st_data[nox].batasH  = aH;
		st_data[nox].batasHH = aHH;
		st_data[nox].rangeH = rH;
	}
	
	if(flag_data==2 && flag_save_data==1 && flag_cpy==1){
		simpan_st_rom(SEKTOR_DATA, lok, 1, (unsigned short *) st_data, 1);
		vPortFree (st_data);
		flag_save_data=0;
		lok=0;
		flag_cpy=0;
	} 

}

int flag_save_file=0;
struct t_file *st_file;

int parsing_cfg_files(int i, char *str)	{
	int nx, num, flag_file=0;
	char tstr[32], *pch;
	float rL, aLL, aL, aH, aHH, rH;
	
	pch = strchr(str, '=') + 1;		//if (pch==NULL)	return 1;
	pch = strtok(pch, "\r\n");		//if (pch==NULL)	return 2;
	
	//struct t_file *st_file;
	if (i==FILES_FLAG)	{
		sscanf(str, "flag = %d", &nx);
		uprintf("   FILE flag:%d\r\n", nx);
		flag_file = nx;
	}
	
	if(flag_file==1 ){  //1: start env , 2: end env, 0 : unknown
		
		if(flag_save_file==1){
			vPortFree(st_file);
		}
		
		st_file = pvPortMalloc( sizeof (struct t_file) );
		if (st_file == NULL)	{
			uprintf(" %s(): ERR allok memory gagal !\r\n", __FUNCTION__);
			vPortFree (st_file);
			return 2;
		}
		#ifdef debug
		printf("  %s(): Mallok @ %X\r\n", __FUNCTION__, st_file);
		#endif
		memcpy((char *) st_file, (char *) ALMT_FILE, (sizeof (struct t_file)));
		
		
		
		flag_save_file=1;
	}
	
	
	if (i==FILES_JUMLAH)	{
		sscanf(str, "jumlah = %d", &nx);
		do	{
			pch++;
		} while (*pch==' ');
		#ifdef debug
		uprintf("   FILES jml:%d\r\n", nx);
		#endif
		st_file->jml = nx;
	}
	else if (i==FILES_URUT)	{
		sscanf(str, "urut%d = ", &nx);
		num = atoi(pch);
		
		if (num>0)	{
		#ifdef debug
		//if ((num>0) && ())	
			uprintf("   FILES [%2d] urut:%d\r\n", nx, num);
		#endif
			st_file->urut[nx-1] = num;
		}
	}
	
	if(flag_file==2 && flag_save_file==1){
		simpan_st_rom(SEKTOR_ENV, BERKAS, 1, (unsigned short *) st_file, 0);
		vPortFree (st_file);
		flag_save_file=0;
	} 
	
}

int parsing_cmd_setting_subutama(char *str)	{
	int jmlIs, nn=-1, i, nx;
	char *pch, *pch2, *pch3, *pch4;
	
	if (st_conf.cmd==CONF_ENV)	{
		jmlIs = nIstilah(strcmd_env);
		for(i=0; i<jmlIs; i++)	{
			pch = strstr (str, strcmd_env[i]);
			if (pch != NULL)	{
				parsing_cfg_env(i, str);
			}
		}
	}
	
	else if (st_conf.cmd==CONF_KANAL)	{
		jmlIs = nIstilah(strcmd_kanal);
		for(i=0; i<jmlIs; i++)	{
			pch = strstr (str, strcmd_kanal[i]);
			if (pch != NULL)	{
				parsing_cfg_kanal(i, str);
			}
		}
	}
	else if (st_conf.cmd==CONF_SUMBER)	{
		jmlIs = nIstilah(strcmd_smbr);
		for(i=0; i<jmlIs; i++)	{
			pch = strstr (str, strcmd_smbr[i]);
			if (pch != NULL)	{
				parsing_cfg_sumber(i, str);
			}
		}
	}
	else if (st_conf.cmd==CONF_DATA)	{
		jmlIs = nIstilah(strcmd_data);
		for(i=0; i<jmlIs; i++)	{
			pch = strstr (str, strcmd_data[i]);
			if (pch != NULL)	{
				parsing_cfg_data(i, str);
			}
		}
	}
	else if (st_conf.cmd==CONF_FILES)	{
		jmlIs = nIstilah(strcmd_files);
		for(i=0; i<jmlIs; i++)	{
			pch = strstr (str, strcmd_files[i]);
			if (pch != NULL)	{
				parsing_cfg_files(i, str);
			}
		}
	}
	
	#ifdef debug
	printf ("----- %d/%d, %s", st_conf.cmd, jmlIs, str);
	#endif
	return 0;
}

void parsing_file_setting(char *str)	{
	char *pch, *pch2;
	int n;
	
	pch = strchr(str, '#');		// komentar
	if (pch != NULL)	{
		n = pch-str+1;
		if (n<10)	{
			#ifdef debug
			printf(" n: %d, komentar; %s", n, str);
			#endif
			return;
		}
	}
	
	pch = strchr(str, '[');
	if (pch != NULL)	{
		vTaskDelay(1);
		parsing_cmd_setting_utama(str);
		#ifdef debug
		uprintf ("NOT NILL cmd: %d\r\n", st_conf.cmd);
		#endif
	} else {
		if(strstr(str,"**END**")==NULL)
		parsing_cmd_setting_subutama(str);
		else
		return;
	}
}

int upload_konfig(char *path)	{
	FIL filx;
	FRESULT res;
	char strfile[128];
	char bufx[2];
	
	if( xSemSD != NULL )    {
	//printf("xSemSD = OK\r\n");
	
		if( xSemaphoreTake( xSemSD, ( portTickType ) 1000 ) == pdTRUE )	{
			//f_close(&filx);
			mundurkan_path();
			res = f_open(&filx, path,  FA_READ );
			if (res != FR_OK)	{
				uprintf("=== TIDAK BISA BUKA FILE ====\r\n");
				fs_remount();
				reset_path();
				xSemaphoreGive( xSemSD );
				return 1;
			}

			/*while ( f_gets (strfile , 128 , &filx) != NULL )	{
				parsing_file_setting(strfile);
				vTaskDelay(300);
				//if (baris==20) break;
				//baris++;
				//memset(strfile,0,sizeof(strfile));
			}
			*/
			
			//ret = (sizeof (strfile))-16;
			memset(strfile,'\0',sizeof(strfile));
			bufx[1]='\0';
			unsigned int ln=0;
			for (;;)	{
				f_read( &filx, bufx, 1, &ln);
				printf("%c", bufx[0]);
				
				if(bufx[0]=='\n'){
					parsing_file_setting(strfile);
					memset(strfile,'\0',sizeof(strfile));
				
				}else{
					
					strcat(strfile,bufx);
				}
				if (ln < 1) break; // sudah mencapai akhir file		
			}
			//f_close( &fd );
			
			uprintf("=== AKHIR FILE ====\r\n");
			f_close(&filx);
			#ifdef debug
			uprintf("=== TUTUP FILE ====\r\n");
			#endif
			xSemaphoreGive( xSemSD );
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

char *pisah_nf(char *pnf)	{
	char *pch, pcx[64], stmp[128];
	
	strcpy(stmp, pnf);
	printf("--> %s() path: %s\r\n", __FUNCTION__, stmp);
	pch = strtok (stmp,"\\");
	while (pch != NULL)  {
		strcpy(pcx, pch);
		pch = strtok (NULL, "\\");
	}
	printf("hasil: %s\r\n", pcx);
	return pcx;
}

void sendHexFile(int nilai, int jml, FIL fp)	{
	int i;
	char s[4];
	memcpy(s, (void*) &nilai, jml);
	f_write(&fp, s, jml, &i);
	
	#if 0
	for (i=0; i<jml; i++)	{
		fputc(s[i], fp);
	}
	#endif
}

int cek_sd(){
	
	if(FIO3PIN & BIT(INS_SDC))
	return 1;
	else
	return 0;
	
}


int loop2=0; 

int reload=0;
int hasil=0;

void simpan_file_data()	{
	struct t_env *st_env;
	st_env=ALMT_ENV;
	int d_f=0;
	
	if(st_env->prioDebug==11){
		d_f=1;
	}else{
		d_f=0;
	}
	
	FIL filx;
	FRESULT res;
	DIR dir;
	FATFS FFS;
	FILINFO finfo;
	char st[50], s[4], fol[20];
	char isi[256];
	time_t wkt;
	struct tm *a;
	int i=0, oo, menit;
	unsigned int wx;
	int j, k;
	
	rtcCTIME0_t ctime0;
	rtcCTIME1_t ctime1;
	ctime0.i = RTC_CTIME0; 
	ctime1.i = RTC_CTIME1; 
	
	st_hw.wkt_now=(unsigned int) now_to_time(1, a) +(3600*st_env->UTC);
	a = localtime (&st_hw.wkt_now);
	
	uprintf("waktu: %d%02d%02d_%02d%02d\r\n", ctime1.year, ctime1.month, ctime1.dom, a->tm_hour, a->tm_min );
	menit =  (ctime0.minutes<30)?0:30;
	sprintf(fol, "0:\\%d%02d%02d", ctime1.year, ctime1.month, ctime1.dom);
	sprintf(st, "%s\\%02d%02d.str", fol,  a->tm_hour, menit);
	
	if(d_f==1){
		printf("filename : %s \r\n",st);
		printf("dir : %s \r\n",fol);
	}
	
	res=f_stat(st,&finfo);
		
	if(res==FR_OK){
		if(d_f==1){
			printf("Size: %lu\n", finfo.fsize);
		}
			
	}else if(res==FR_NO_PATH){ //dir ga ada
		if(d_f==1){
			printf("dir not exist, res: %d",res);
		}
	
		res = f_mkdir(fol);
			
		if(res==FR_OK){
			if(d_f==1){
				printf("res buat folder: %d\r\n", res);
			}
		}else{
			fs_remount();
			return;
		}
	
	}else if(res==FR_NO_FILE){
		if(d_f==1){
			printf("no file: %d\r\n", res);
		}
			
	}else{
		if(d_f==1){
			printf("fstat error: %d\r\n", res);
		}
	}
		
	//hasil=res;
	
	#if 0
	res = f_opendir(&dir, fol);
	
	if(d_f==1){
		printf("open folder %s: %d\r\n", fol,res);
	}
	
	if(res){
		
		//sRelay1();
		//cRelay2();
		/*if (f_mount(0,"0:",1) == FR_OK)	{
			printf("%s():unmount OK\r\n", __FUNCTION__);
			//st_hw.sdcmounted=1;
		}
		else	{
			printf("%s():unmount ERROR\r\n", __FUNCTION__);
			return;
		}
		* */
		fs_remount();
		//vPortFree(isi);
		if(d_f==1){
			printf("open dir error %d\r\n",res);
		}
		return;
	}
	
	#endif
		
	struct t_file  *st_file;
	st_file = (char *) ALMT_FILE;
	
	struct t_data *st_data;
	
	res = f_open(&filx, st, FA_READ | FA_WRITE | FA_OPEN_APPEND);

	if(d_f==1){
		printf("open apend, res : %d\r\n", res);
	}

	if (res==FR_OK){
	  	res=f_sync(&filx);
	  	
	  	if(d_f==1){
			printf("res sync 1 : %d\r\n",res);
		}
		
	}else{
	  	if(d_f==1){
			printf("open file error %d\r\n",res);
		}
		
		fs_remount();
		return;
	 }
	
	memset(st,'\0', sizeof(st));
	
	f_printf(&filx, "SN,%s,", st_env->SN);	
	f_printf(&filx, "TIME,%ld", now_to_time(1, a));
	
	for (i=0; i<st_file->jml; i++)	{
		j = (int) ((st_file->urut[i])/PER_SUMBER);
		k = (st_file->urut[i]) % PER_SUMBER;
		st_data = ALMT_DATA + j*JML_KOPI_TEMP;
	
		memset(isi,'\0', sizeof(isi));
		sprintf(isi, ",%d,%f", st_data[k].id, data_f[st_file->urut[i]]);
		res=f_puts(isi,&filx);
	}
	
	f_printf(&filx,",GPS");
	
	for(i=0;i<JML_DATA_GPS;i++){	
		
		memset(isi,'\0', sizeof(isi));
		sprintf(isi, ",%f",  data_gps[i]);
		res=f_puts(isi,&filx);

	}
	
	res=st_file->jml+JML_DATA_GPS;
	sprintf(isi,",j=%d\n",res);
	res=f_puts(isi,&filx);
	
	if(d_f==1){
		printf("res fputs akhir :%d", res);
	}
	
	portENTER_CRITICAL();
	res=f_sync(&filx);
	portEXIT_CRITICAL();
	
	if(d_f==1){
		printf("res sync akhir :%d", res);
	}
	
	res=f_close(&filx);
	
	if(d_f==1){
		printf("res close akhir :%d", res);
	}
}



FIL filz;
FRESULT res,res2;
DIR dirz;

abs_fil[50];
int opened;

int close_file_simpan(){
	int result;
	if(opened=1)
	opened=0;
	result = f_close(&filz);
	
	return result; 	
}

int reset_path(){
	memset(abs_path,'\0',20);
	return 1;
	
}

/*
void tulis_konfig_file(char *s, FIL* fp)	{
	//char ss[50];
	//int jml, i;
	
	//sprintf(ss, "%s\r\n", s);
	//jml = strlen(ss);
	#ifdef debug
	//uprintf("%s: jml: %d\r\n", ss, jml);
	#endif
	f_puts(s,&fp);
	
}
*/

#define tulis_konfig_file f_puts

int simpan_konfig(int argc, char **argv)		{
	FIL filx;
	FRESULT res;
	char st1[50], st2[50], s[4];
	int i, j, oo;
	
	if (argc==1)	{
		//file_kitab(0);
		uprintf("\r\n  Gagal Simpan Konfig, Nama File Belum Diinput (contoh: simpan_konfig monita.str)\r\n");
		return;
	}
	
	if(st_hw.sdc !=1){
		uprintf("\r\n  Gagal Simpan Konfig, SDCard Not Found/ ERROR\r\n");
		return;
	}
	//uprintf("Proses  : %s\r\n", argv[1]);
	
	if( xSemSD != NULL )    {
		//printf("xSemSD = OK\r\n");
		
		if( xSemaphoreTake( xSemSD, ( portTickType ) 1000 ) == pdTRUE )	{
			//printf("semaphore take\r\n");
			//simpan_file_data2();
			//simpan_file_data();
			
			//printf("semaphore give\r\n");
		
		}else{
			printf("gagal simpan konfig : SDCard Busy, Silahkan Coba beberapa saat lagi\r\n");
			//xSemaphoreGive( xSemSD);
			return;
		}
		
	}else{
		//xSemaphoreGive( xSemSD);
		return;
	}
	rtcCTIME0_t ctime0;
	rtcCTIME1_t ctime1;
	ctime0.i = RTC_CTIME0; 
	ctime1.i = RTC_CTIME1; 
	
	sprintf(st2, "%d%02d%02d %02d:%02d:%02d", \
		ctime1.year, ctime1.month, ctime1.dom, ctime0.hours, ctime0.minutes, ctime0.seconds);
	sprintf(st1,"0:\\%s",argv[1]);	
	res = f_open(&filx, st1, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
    if (res) {
		uprintf("\r\n  res: %d\r\n", res);
		xSemaphoreGive( xSemSD);
		return;
		//die(res);
	}
	
	res = f_sync(&filx);
	
	struct t_env *st_env;
	st_env = (char *)ALMT_ENV;
	
	#ifdef debug
	uprintf("--> %s\r\n", __FUNCTION__);
	#endif
	tulis_konfig_file("[env]\n", &filx);
	sprintf(st1, "#tanggal = %s\n", st2);
	tulis_konfig_file(st1, &filx);
	tulis_konfig_file("flag = 1\n", &filx);
	sprintf(st1, "nama = %s\n", st_env->nama_board);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "sn = %s\n", st_env->SN);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "server = %d.%d.%d.%d\n", st_env->wIP0, st_env->wIP1, st_env->wIP2, st_env->wIP3);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "domain = %s\n", st_env->domain);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "domain_mode = %d\n", st_env->domain_mode);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "port = %d\n", st_env->Port);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "file = %s\n", st_env->berkas);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "kirim = %d\n", st_env->statusWebClient);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "mode_kirim = %d\n", st_env->mode_kirim);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "int_data = %d\n", st_env->intReset);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "int_relay = %d\n", st_env->intervalkirim);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "gps_en = %d\n", st_env->gps_en);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "id_r1 = %d\n", st_env->idrelay1);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "id_r2 = %d\n", st_env->idrelay2);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "utc = %d\n", st_env->UTC);
	tulis_konfig_file(st1, &filx);
	
	sprintf(st1, "idslave = %d\n", st_env->almtSlave);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "debug1 = %d\n", st_env->prioDebug);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "debug2 = %d\n", st_env->prioDebug2);
	tulis_konfig_file(st1, &filx);
	sprintf(st1, "#board = %s %s\n", PROMPT, BOARD_SANTER_versi);
	tulis_konfig_file(st1, &filx);
	tulis_konfig_file("", &filx);
	
	tulis_konfig_file("[kanal]\n", &filx);
	for (i=0; i<(JML_KANAL*2); i++)	{
		sprintf(st1, "kalib%d = %.3f %.3f\n", i+1, st_env->kalib[i].m, st_env->kalib[i].C);
		tulis_konfig_file(st1, &filx);
		sprintf(st1, "status%d = %d\n", i+1, st_env->kalib[i].status);
		tulis_konfig_file(st1, &filx);
	}
	
	tulis_konfig_file("flag = 2\n", &filx);
	
	tulis_konfig_file("\n", &filx);
	
	struct t_sumber *st_sumber;
	st_sumber = (char *) ALMT_SUMBER;
	
	tulis_konfig_file("[sumber]\n", &filx);
	tulis_konfig_file("flag = 1\n", &filx);
	
	for (i=0; i<JML_SUMBER; i++)	{
		sprintf(st1, "nama%d = %s\n", i+1, st_sumber[i].nama);
		tulis_konfig_file(st1, &filx);
		sprintf(st1, "ip%d = %d.%d.%d.%d\n", i+1, st_sumber[i].IP0, st_sumber[i].IP1, st_sumber[i].IP2, st_sumber[i].IP3);
		tulis_konfig_file(st1, &filx);
		sprintf(st1, "alamat%d = %d\n", i+1, st_sumber[i].alamat);
		tulis_konfig_file(st1, &filx);
		sprintf(st1, "stack%d = %d\n", i+1, st_sumber[i].stack);
		tulis_konfig_file(st1, &filx);
		sprintf(st1, "formula%d = %d\n", i+1, st_sumber[i].form);
		tulis_konfig_file(st1, &filx);
	}
	tulis_konfig_file("flag = 2\n", &filx);
	
	tulis_konfig_file("\n", &filx);
	
	struct t_data *st_data;
	tulis_konfig_file("[data]\n", &filx);
	for (i=0; i<JML_SUMBER; i++)	{
		st_data = ALMT_DATA + i*JML_KOPI_TEMP;
		tulis_konfig_file("flag = 1\n", &filx);
		sprintf(st1, "no = %d\n", i+1);
		tulis_konfig_file(st1, &filx);
		
		for (j=0; j<PER_SUMBER; j++)	{
			sprintf(st1, "nama%d = %s\n", i*PER_SUMBER+j+1, st_data[j].nama);
			tulis_konfig_file(st1, &filx);
			sprintf(st1, "id%d = %d\n", i*PER_SUMBER+j+1, st_data[j].id);
			tulis_konfig_file(st1, &filx);
			sprintf(st1, "satuan%d = %s\n", i*PER_SUMBER+j+1, st_data[j].satuan);
			tulis_konfig_file(st1, &filx);
			sprintf(st1, "status%d = %s\n", i*PER_SUMBER+j+1, st_data[j].status?"Aktif":"Mati");
			tulis_konfig_file(st1, &filx);
			sprintf(st1, "batas%d = %d %d %d %d %d %d\n", i*PER_SUMBER+j+1, st_data[j].rangeL, \
					st_data[j].batasLL, st_data[j].batasL, 	\
					st_data[j].batasH, st_data[j].batasHH, st_data[j].rangeH);
			tulis_konfig_file(st1, &filx);
			sprintf(st1, "formula%d = %s\n", i*PER_SUMBER+j+1, st_data[j].formula);
			tulis_konfig_file(st1, &filx);
		}
		tulis_konfig_file("flag = 2\n", &filx);
	
	}
	tulis_konfig_file("\n", &filx);
	
	struct t_file  *st_file;
	st_file = (char *) ALMT_FILE;
	tulis_konfig_file("[files]\n", &filx);
	tulis_konfig_file("flag = 1\n", &filx);
	
	sprintf(st1, "jumlah = %d\n", st_file->jml);
	tulis_konfig_file(st1, &filx);
	
	for (i=0; i<JML_TITIK_DATA; i++)	{
		if (st_file->urut[i]==0)	break;
		sprintf(st1, "urut%d = %d\n", i+1, st_file->urut[i]);
		tulis_konfig_file(st1, &filx);
	}
	tulis_konfig_file("flag = 2\n", &filx);
	
	tulis_konfig_file("**END**\n", &filx);
	
	f_sync(&filx);
	f_close(&filx);
	printf("Seselai Simpan Konfig : %s\r\n", argv[1]);
	xSemaphoreGive( xSemSD);
}

void cari_waktu(char *dest, char *posisi) {		// cari posisi path folder
	
	if ((posisi[0]!='H') && (posisi[0]!='h') && (posisi[0]!='J') && (posisi[0]!='j') && (posisi[0]!='B') && (posisi[0]!='b') ) {
		uprintf("Argumen tidak benar !!\r\n");
		uprintf("Contoh : H-7, J-2, B-1\r\n");
		sprintf(dest,"\\");
		return;
	}

	struct tm *a;
	unsigned int wx = (unsigned int) now_to_time(1, a);		// epoch
	#ifdef debug
	uprintf("epoch : %ld\r\n", wx);
	#endif
	char *pch, str[10];
	pch=strstr(posisi,"-");
  	if (pch!=NULL)
  		strcpy(str, pch+1);
  	else
  		return;
	
	#if 0
	uprintf("waktu: %d%02d%02d_%02d%02d\r\n", ctime1.year, ctime1.month, ctime1.dom, ctime0.hours, ctime0.minutes );
	#endif
	int ijam, itgl, ibulan, ithn, count;

	if ( (posisi[0]=='H') || (posisi[0]=='h') ) {
		wx -= (24*3600*atoi(str));
	}
	if ( (posisi[0]=='J') || (posisi[0]=='j') ) {
		wx -= (3600*atoi(str));
	}
	
	a = localtime (&wx);
	#ifdef debug
	uprintf("tmp: %d, time: %ld, %04d-%02d-%02d %02d:%02d\r\n", \
		atoi(str), wx, a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour, a->tm_min);
		#endif
	//
	sprintf(dest, "\\%04d%02d%02d", a->tm_year+1900, a->tm_mon+1, a->tm_mday);
}

int baca_waktu_data_sd(){
	
	return 0;
}

int cari_files (char* pathxx, char *nf, int aksi) {
	//char buf_lfn[255];
	FRESULT res;
	FILINFO fnoxx;
	DIR dirs;
	char *nama;
	int i=0;
	static char aaaa[64];
	static char bbbb[255];
	strcpy(aaaa, pathxx);
	unsigned int jum_dirs=0;
	//fno.lfname = buf_lfn;
	//fno.lfsize = 255;//sizeof (buf_lfn);
	static char lfnxx[_MAX_LFN * (1) + 1];
    strcpy(fnoxx.fname,lfnxx);
    fnoxx.fsize = sizeof(lfnxx);
    
	if ((res = f_opendir (&dirs,  pathxx)))		{ 
		#ifdef debug
		uprintf("%s(): ERROR = %d\r\n", __FUNCTION__, res);
		#endif
		return -1;
	}
	#ifdef debug
	printf("%s(): Open dir %s OK\r\n", __FUNCTION__, pathxx);
	#endif
	char waktu[64];
	struct tm *a;
	unsigned int wx = (unsigned int) now_to_time(1, a);		// epoch
	a = localtime (&wx);
	sprintf(waktu, "%04d%02d%02d_%02d", a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour);
	#ifdef debug
	uprintf("=====> file waktu : %s\r\n", waktu);
	#endif
	if (res == FR_OK) {
		for(;;) {
			res = f_readdir(&dirs, &fnoxx);
			
			if (res != FR_OK || fnoxx.fname[0] == 0) break;
			if (fnoxx.fname[0] == 0)
				nama = &(fnoxx.fname [0]);
			else
				nama = &(fnoxx.fname[0]);

			//sprintf(bbbb,"%s\\%s", aaaa, nama);		//
			//sprintf(bbbb,"%s\\",aaaa);
			strcpy(bbbb,aaaa);
			strcat(bbbb,"\\");
			strcat(bbbb,nama);
			//printf("path: %s, %s, nama: %s\r\n",aaaa, bbbb, nama);
			/*
			printf ("\r\n%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s",
					(fnoxx.fattrib & AM_DIR) ? 'D' : '-',
					(fnoxx.fattrib & AM_RDO) ? 'R' : '-',
					(fnoxx.fattrib & AM_HID) ? 'H' : '-',
					(fnoxx.fattrib & AM_SYS) ? 'S' : '-',
					(fnoxx.fattrib & AM_ARC) ? 'A' : '-',
					(fnoxx.fdate >> 9) + 1980, (fnoxx.fdate >> 5) & 15, fnoxx.fdate & 31,
					(fnoxx.ftime >> 11), (fnoxx.ftime >> 5) & 63,
					fnoxx.fsize, nama);
			//*/
			//*
			
			
			if (strncmp(waktu, nama, 11))	{		// 
				if (aksi == LIHAT)	{
					//uprintf("aksi: %d, path: %s, namafile: %s\r\n", aksi, bbbb, nama);
					i++;
				}
				if (aksi == LIHAT_ISI_SATU)	{
					//uprintf("aksi: %d, path: %s, namafile: %s\r\n", aksi, bbbb, nama);
					sprintf(pathxx, "%s", bbbb);
					sprintf(nf, "%s", nama);
					return 1;
				}
			}
			//*/
		}
	}
	return i;
}

int hapus_folder(char *fol)	{
	f_unlink(fol);
}

int cari_berkas(char *str_doku, char *path, int aksi) {
	char c = str_doku[0];
	if ((c!='H') && (c!='h') && (c!='J') && (c!='j') && (c!='B') && (c!='b') ) {
		printf("Argumen tidak benar !!\r\n");
		printf("Contoh : H-7, J-2, B-1\r\n");
		return -1;
	}
	
	char path_bk[127], namafile[64];
	char *pch, str[10], waktu[10];
	int i=0,j;
	strcpy(waktu,str_doku);
	pch=strstr(waktu,"-");
  	if (pch!=NULL)
  		strcpy(str, pch+1);

	#ifdef debug
	printf("str %s: %d\r\n", __FUNCTION__, atoi(str));
  	#endif
  	for(i=atoi(str); i>=0; i--) {
		sprintf(waktu, "%c-%d", waktu[0],i);
		cari_waktu(path_bk, waktu);
		j=cari_files(path_bk, namafile, aksi);
		#ifdef debug
		uprintf("_______________j:%d, waktu: %s, path: %s\r\n", j, waktu, path_bk);
		#endif
		if (j==0)	hapus_folder(path_bk);
		if (aksi==LIHAT_ISI_SATU && j>0)	break;
		if (j==90)	break;
	}
	
	if (aksi==LIHAT_ISI_SATU)	{
		sprintf(path, "%s", path_bk);
	}
	
	return j;
	//*/
}

#endif
