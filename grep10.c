/*
 	Valeria Gonzalez
 */
 #include <stdio.h>
 #include <signal.h>
 #include <setjmp.h>
 #include <string.h>
 #include <stdlib.h>
 #include <ctype.h>
 #include "grep.h"

#define	BLKSIZE	4096
#define	NBLK	2047

#define	FNSIZE	128
#define	LBSIZE	4096
#define	ESIZE	256
#define	GBSIZE	256
#define	NBRA	5
#define	KSIZE	9


char	Q[]	= "";
char	T[]	= "TMP";
#define	READ	0
#define	WRITE	1

int	peekc;
int	lastc;
char	savedfile[FNSIZE];
char	file[FNSIZE];
char	linebuf[LBSIZE];
char	rhsbuf[LBSIZE/2];
char	expbuf[ESIZE+4];
int	given;
unsigned int	*addr1, *addr2;
unsigned int	*dot, *dol, *zero;
char	genbuf[LBSIZE];
long	count;
char	*nextip;
char	*linebp;
int	ninbuf;
int	io;
int	pflag;
long	lseek(int, long, int);
int	open(char *, int);
int	creat(char *, int);
int	read(int, char*, int);
int	write(int, char*, int);
int	close(int);
int	fork(void);
int	execl(char *, ...);
void	exit(int);
int	wait(int *);
int	unlink(char *);


int	vflag	= 1;
int	oflag;
int	listf;
int	listn;
int	col;
char	*globp;
int	tfile	= -1;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	ibuff[BLKSIZE];
int	iblock	= -1;
char	obuff[BLKSIZE];
int	oblock	= -1;
int	ichanged;
int	nleft;
char	WRERR[]	= "WRITE ERROR";
int	names[26];
int	anymarks;
char	*braslist[NBRA];
char	*braelist[NBRA];
int	nbra;
int	subnewa;
int	subolda;
int	fchange;
int	wrapp;
int	bpagesize = 20;
unsigned nlall = 128;
_Bool isArray = 0;
_Bool noCommand = 0;


jmp_buf	savej;

int main(int argc, char *argv[]) {
	int f = getfile();
	printf("%d\n", f);
	int count = 0;

	argv++;
	for (int i = 0 ; i < argc ; i++) {
    if ((i > 1 && strrchr(argv[1],'-') == NULL) || i > 2) {
      strcpy(&savedfile[count],argv[i]);
      count++;
    }
  }
	if (argc >= 3) {
      if (argc == 4 ) {
        strncpy(ibuff, argv[2], sizeof(ibuff));
        strncpy(file, argv[3], sizeof(file));
				searchFileinDirectory(file, savedfile);
      }
        if (argc > 4) {
          strncpy(ibuff, argv[2], sizeof(ibuff));
          isArray = 1;

          for (int i = 0 ; i < count ; i++){
            strncpy(file, &savedfile[i], sizeof(file));
						searchFileinDirectory(file, savedfile);
            printf("\n");
          }
    		}
        else{
            print();
				}
      }
      else if (argc == 3) {
        noCommand = 1;
        strncpy(ibuff, argv[1], sizeof(ibuff));
        strncpy(file, argv[2], sizeof(file));
				searchFileinDirectory(file, savedfile);
      }
      else {
        if (strrchr(argv[1],'-') != NULL) {
          strncpy(ibuff, argv[2], sizeof(ibuff));
        }
        else {
          strncpy(ibuff, argv[1], sizeof(ibuff));
        }
        isArray = 1;
        for (int i = 0 ; i < count ; i++){
          strncpy(file, &savedfile[i], sizeof(file));
          printf("\n");
        }
				print();
      }

    return 0;
}

void print(void) {
	unsigned int *a1;

	a1 = addr1;
	do {
		if (listn) {
			count = a1-zero;
			putchar('\t');
		}
	} while (a1 <= addr2);
	dot = addr2;
	listf = 0;
	listn = 0;
	pflag = 0;
}

void filename(int comm) {
	char *p1, *p2;
	int c;

	count = 0;
	c = getchar();
	if (c=='\n' || c==EOF) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			error(Q);
		p2 = file;
		while (*p2++ == *p1++)
			;
		return;
	}
	if (c!=' ')
		error(Q);
	while ((c = getchar()) == ' ')
		;
	if (c=='\n')
		error(Q);
	p1 = file;
	do {
		if (p1 >= &file[sizeof(file)-1] || c==' ' || c==EOF)
			error(Q);
		*p1++ = c;
	} while ((c = getchar()) != '\n');
	*p1++ = 0;
	if (savedfile[0]==0 || comm=='e' || comm=='f') {
		p1 = savedfile;
		p2 = file;
		while (*p1++ == *p2++)
			;
	}
}

void error(char *s) {
	int c;

	wrapp = 0;
	listf = 0;
	listn = 0;
	putchar('?');
	puts(s);
	count = 0;
	lseek(0, (long)0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getchar()) != '\n' && c != EOF)
			;
	if (io > 0) {
		close(io);
		io = -1;
	}
	longjmp(savej, 1);
}

int getfile(void) {
	int c;
	char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0){
				if (lp>linebuf) {
					puts("'\\n' appended");
					*genbuf = '\n';
				}
				else {
					return(EOF);
				}
			}
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) {
				if (*fp++ & 0200)
					break;
			}
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0')
			continue;
		if (c&0200 || lp >= &linebuf[LBSIZE]) {
			lastc = '\n';
			error(Q);
		}
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

void putfile(void) {
	unsigned int *a1;
	int n;
	char *fp, *lp;
	int nib;

	nib = BLKSIZE;
	fp = genbuf;
	a1 = addr1;
	do {
		for (;;) {
			if (--nib < 0) {
				n = fp-genbuf;
				if(write(io, genbuf, n) != n) {
					puts(WRERR);
					error(Q);
				}
				nib = BLKSIZE-1;
				fp = genbuf;
			}
			count++;
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	n = fp-genbuf;
	if(write(io, genbuf, n) != n) {
		puts(WRERR);
		error(Q);
	}
}

void quit(int n) {
	if (vflag && fchange && dol!=zero) {
		fchange = 0;
		error(Q);
	}
	unlink(tfname);
	exit(0);
}

int putline(void) {
	char *bp, *lp;
	int nl;
	unsigned int tl;

	fchange = 1;
	lp = linebuf;
	tl = tline;
	nl = nleft;
	tl &= ~((BLKSIZE/2)-1);
	while (*bp == *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

void searchFileinDirectory(char *filepath, char *file) {
  char line[512];
  int lineNum = 0;
  _Bool isFile = 0;
  int count = 0;

  FILE *f = fopen(filepath, "r");

  if(f == NULL) {
    printf("File %s does not exist.\n", filepath);
  }
  else {

    FILE *outfile = fopen("output.txt", "w");

    while (fgets(line, 1024, f)) {
      fprintf(outfile, "%d:\t%s\n", lineNum, file);
      strcpy(line, file);
        for (int i = 0 ; i < strlen(file) ; i++) {
					for(int j = 0; j < strlen(savedfile); j++){
            if(line[i] != savedfile[j])
							break;
					}
        }

      if(strstr(line, ibuff)) {
          printf("%s: ", file);
        }
    }
    if (isFile)
      printf("%s\n", file);
    puts("");
    fclose(outfile);
  	fclose(f);
}
}
