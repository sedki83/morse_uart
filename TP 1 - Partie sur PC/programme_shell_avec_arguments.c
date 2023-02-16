/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

#include <string.h>

#include <termios.h> // Contains POSIX terminal control definitions

#include <fcntl.h> // Contains file controls like O_RDWR

void main (int argc, char **argv)
{
	int serial_port = open("/dev/ttyS3", O_RDWR);

    // Check for errors
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    //tty.c_cflag |= PARENB;  // Set parity bit, enabling parity

    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    //tty.c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication

    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    //tty.c_cflag |= CS5; // 5 bits per byte
    //tty.c_cflag |= CS6; // 6 bits per byte
    //tty.c_cflag |= CS7; // 7 bits per byte
    tty.c_cflag |= CS8; // 8 bits per byte (most common)

    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    //tty.c_cflag |= CRTSCTS;  // Enable RTS/CTS hardware flow control

    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON; // diseable canonical mode
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    //tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    //tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetspeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }




	int c;
	extern char *optarg;
	extern int optind;

	int timer1_ms = 0; 
	int timer2_ms = 0; 
	int timer3_ms = 0;

	char message[100] = "";
	int n;
	int iteration =1 ;
	int boucle =0;
	int msgLen =0;

	while ((c = getopt(argc, argv, "t:n:bhm:s")) != -1) {
		switch (c) {
            case 't':
				timer1_ms = atoi(strtok(optarg, " "));
				timer2_ms = atoi(strtok(NULL, " "));
				timer3_ms = atoi(strtok(NULL, " "));

				printf("Temps définis : court = %dms, moyen = %dms, long = %dms.\n", timer1_ms, timer2_ms, timer3_ms);

                break;

			case 'm':

				strcpy(message, optarg);
				printf("%s\n", message);
				msgLen = strlen(message);
				break;

			case 's':
				printf("A bientot");
				exit(2);
			
			case 'b':
                if (iteration == 1)
                {
                    boucle = 1;
                    iteration = 0;
                }
                else {
                    printf("L'option -b ne peut pas etre utilisee en meme temps que l'option -n\n");
                }
                break;
            case 'n':
                if (boucle == 0)
                {
                    iteration = atoi(optarg);
					for (int i = 0; i < iteration; i++)
					{
						printf(" %d : Le message sera repete %d fois\n",i, iteration);
					}
					
                    
                }
                else {
                    printf("L'option -n ne peut pas etre utilisee en meme temps que l'option -b\n");
                }
                break;
            case 'h':
            default:
                printf("Utilisation du programme :\n");
                printf("-h : affiche l'aide\n");
                printf("-s : stop le programme\n");
                printf("-m \"xxx\" : message à envoyer\n");
                printf("-t \"# # #\" : configure les timers avec les temps indiqués\n");
                printf("-b : message affiché en morse en boucle\n");
                printf("-n # : message affiché en morse n-fois\n");

				exit(1) ;
                break;
		}
	} 


	if (strcmp(message, "") == 0) {
		printf("Précisez un message '-m' ou envoyez la commande stop '-s'. Option '-h' pour l'aide.\n");
		exit(0);
	}

	int tram_length = msgLen + 7;

	unsigned char msg[tram_length] ;

	msg[0] = boucle;
	msg[1] = iteration;
	msg[2] = timer1_ms;
	msg[3] = timer2_ms;
	msg[4] = timer3_ms;
	msg[5] = msgLen;

    for (int i = 0; i < msgLen; i++)
    {
        msg[(6+i)] = message[i];
    }
    msg[(tram_length - 1)] = '\n';
    

	write(serial_port, msg, sizeof(msg));

}

