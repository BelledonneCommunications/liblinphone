/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "../config.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int ifd,ofd;
	char *name,*p;
	char buf[200];
	int len;

	if (argc<2) return -1;	
	name=malloc(strlen(argv[1])+10);
	sprintf(name,"%s",argv[1]);
	p=strstr(name,".raw");
	if (p!=NULL){
		sprintf(p,"%s",".wav\0");
	}else{
		sprintf(name,"%s%s",argv[1],".raw");
	}
	
	ifd=open(name,O_RDONLY);
	if (ifd<0) {
		perror("Could not open input file");
		return -1;
	}
	ofd=open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP);
	if (ofd<0) {
		perror("Could not open output file");
		return -1;
	}
	len=read(ifd,buf,20);
	printf("len=%i\n",len);
	/* erase the wav header */
	if (len>0){
		memset(buf,0,20);
		write(ofd,buf,20);
	}else{
		printf("Error while processing %s: %s\n",argv[1],strerror(errno));
		return -1;
	};

	while ( (len=read(ifd,buf,200))>0){
		#ifdef WORDS_BIGENDIAN	
		for (i=0;i<len/2;i+=2){
			tmp=buf[i];
			buf[i]=buf[i+1];
			buf[i+1]=tmp;
		}
		#endif
		write(ofd,buf,len);
	}

	close(ifd);
	close(ofd);
	return 0;
}


