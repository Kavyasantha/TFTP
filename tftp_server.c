#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include<signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include<sys/wait.h>


#define MAX_BUF_LEN 520
#define MAX_DATA_LEN 512
#define TIME_OUT 1

int char_n_e_x_t = -1;

int t_check(int fds)
{
    fd_set r_s_e_t;
    struct timeval t;
    
    FD_ZERO(&r_s_e_t);
    FD_SET(fds,&r_s_e_t);
    
    t.tv_usec = 0;
    t.tv_sec = TIME_OUT;
    
    return (select(fds+1,&r_s_e_t,NULL, NULL, &t));
        
}

void sig_child_handle(int st)
{
    int p_i_d;
    int s_err_no= errno;
    
    while(( p_i_d=waitpid(-1,NULL,WNOHANG ))>0)
    {
        printf("_child_process_ID %d \n",p_i_d);
    }
    errno =s_err_no;

}

int main(int argc,char *argv[])
{
    
    int soc,num_p_o_r_t,ip_serv,recv_bytes; 
    struct sockaddr_in serv;
    struct sockaddr_in cli;
    socklen_t size_cli;
    struct sigaction sa;
    
    char r_buf[MAX_BUF_LEN];
    char s_t_r[INET_ADDRSTRLEN];
    
    char buf_sent[MAX_BUF_LEN];
    
    int child_handle;
    char data_sent[MAX_DATA_LEN];
    
    
    if (argc != 3) 
    {
        printf("It must be ./server <IP> <port> \n" );
        return 1;
    }   
    
    memset(&serv, 0, sizeof(serv)); 
    memset(r_buf ,0,sizeof(r_buf));
    memset(&cli, 0,sizeof cli);

    ip_serv = atoi(argv[1]);
    num_p_o_r_t = atoi(argv[2]);
    
    serv.sin_family=AF_INET;
    serv.sin_port=htons(num_p_o_r_t);   

    if((soc = socket(AF_INET, SOCK_DGRAM, 0))==-1)
    {
        printf("--error in creating socket\n" );
        if(errno)
        {
            printf("----exited coz of error - %s\n",strerror(errno));
        }
        return 1;
    }
    
    if((bind(soc,(struct sockaddr*)&serv,sizeof(serv)))==-1)
    {
        printf("--error in binding socket\n" );
        if(errno)
        {
            printf("-----exited coz of error %s\n", strerror(errno));
        }
        close(soc);
        return 1;
        
    }

    printf(" --waiting to connect.... \n");
    
    while(1)
    {
         size_cli = sizeof cli;
         if((recv_bytes = recvfrom(soc, r_buf, sizeof(r_buf), 0, (struct sockaddr *)&cli, &size_cli)) ==-1)
       {
            printf("---error receiving from socket.\n" );
        if(errno)
        {
            printf("----exited coz of error : %s\n", strerror(errno));
        }
        return 1;
       }
       
       inet_ntop(AF_INET, &cli.sin_addr, s_t_r, INET_ADDRSTRLEN);
       printf("server connected with client %s \n",s_t_r);
       

       sa.sa_handler = sig_child_handle; 
       sigemptyset(&sa.sa_mask);
       sa.sa_flags = SA_RESTART;
       if (sigaction(SIGCHLD, &sa, NULL) == -1)
           {
            perror("sigaction");
            exit(1);
           }

        if(!fork())
        {
            char name_of_file[] = " ";
            FILE *file_ptr;
            int op_code,m_o_d_e;
            size_t len_name_of_file;
            char temp_name_of_file[10];
                
            op_code = r_buf[1]; 
            strcpy(name_of_file, &r_buf[2]);
            
            len_name_of_file = strlen(name_of_file);
            
            printf("name_of_file is %s \n" ,name_of_file);
            
                if(strcasecmp(&r_buf[len_name_of_file+3] , "netascii" ) == 0)
                     m_o_d_e =1 ;
                else if(strcasecmp(&r_buf[len_name_of_file+3],"octet") == 0)
                     m_o_d_e =2;
                else
                {
                     printf("UNABLE TO UNDERSTAND m_o_d_e ::ILLEGAL OPERATION \n ");
                     memset(buf_sent , 0, sizeof(buf_sent));
                     
                     buf_sent[1] = 5; 
                     buf_sent[3] = 4; 
                     
                     stpcpy(&buf_sent[4], "m_o_d_e NOT SPECIFICIED" );
                     len_name_of_file = strlen("m_o_d_e NOT SPECIFIED");
                     
                    if(sendto(soc,buf_sent,len_name_of_file+5,0,(struct sockaddr *)&cli, size_cli)==-1)
                    {
                         if(errno)
                            printf("----exited coz of error : %s\n", strerror(errno)); 
                         close(soc);
                         return(1); 
                    }
                     
                     
                }
            
 
                 if(( file_ptr = fopen(name_of_file,"r")) == NULL)
                 {
                    printf("FILE_NOT_FOUND \n");
                    memset(buf_sent,0,sizeof(buf_sent));
                    
                    buf_sent[1] = 5; 
                    buf_sent[3] = 1; 
                    
                    strcpy(&buf_sent[4],"FILE_NOT_FOUND");
                    len_name_of_file = strlen("FILE_NOT_FOUND");
                    
                    if(sendto(soc,buf_sent,len_name_of_file+5,0,(struct sockaddr *)&cli, size_cli)<0)
                    {
                        if(errno)
                            printf("Exiting due to error : %s\n", strerror(errno)); 
                         close(soc);
                         return(1);     
                        
                    }
                     
                }
                
                
                if(m_o_d_e == 1)
                {
                    fseek(file_ptr , 0, SEEK_END);
                    int file_len = ftell(file_ptr);
                    printf("file lenght is %d Bytes \n" ,file_len);
                    fseek(file_ptr , 0, SEEK_SET);
                    
                    
                    FILE *new_Temp_File;
                    new_Temp_File = fopen(temp_name_of_file, "w");
                    int ch = 1;
                    
               
                    while(ch != EOF)
                    {
                        if(char_n_e_x_t >= 0)
                        {
                           fputc(char_n_e_x_t,new_Temp_File)    ;
                           char_n_e_x_t = -1;
                           continue;
                        }
                        
                        ch = fgetc(file_ptr);
                        if(ch == EOF)
                        {
                            if(ferror(file_ptr))
                                printf("---error reading :: fgetc");
                            break;
                            
                        }
                        else if(ch=='\n')
                        {
                           ch='\r';
                           char_n_e_x_t = '\n';
                        }
                        else if(ch=='\r')
                        {
                            char_n_e_x_t='\0';
                        }
                        else
                           char_n_e_x_t=-1;
                        fputc(ch,new_Temp_File);
                        
                    }
                    
                    fseek(new_Temp_File, 0,SEEK_SET);
                    file_ptr = new_Temp_File;
                    file_ptr = fopen(temp_name_of_file , "r");
                             
                }
            
            
            close(soc);
                
                serv.sin_port = htons(0);   
                
                
                if((child_handle = socket(AF_INET,SOCK_DGRAM,0))==-1)
                {
                     printf("---error creating child socket\n" );
                     if(errno)
                       printf(" : %s\n", strerror(errno));
                     return 1;
                }
        
                
                if((bind(child_handle,(struct sockaddr*)&serv,sizeof(serv)))<0)
                {
                    printf("---error binding child Socket\n" );
                    if(errno)
                    {
                        printf("----exited coz of error : %s\n", strerror(errno));
                    }
                    close(child_handle);
                    return 1;
                    
                }
                

                unsigned short int sent_block_Num, sent_Num;
                sent_block_Num = 0;
                sent_Num = 0;
                
                unsigned short int block_f_i_n,ack_no;
                block_f_i_n = 0;
                ack_no = 0;
                
                unsigned short int num_recv;
                
                unsigned int f_off_set = 0;
                int read_bytes,t;
                
                int sent_Bytes_Handler,recv_bytes_Handler;
                
                int count_of_time_out = 0;
            
                
            while(1)
            {
                
                memset(buf_sent,0,sizeof(buf_sent));
                sprintf(buf_sent,"%c%c",0x00,0x03); 
                
                    sent_Num = sent_block_Num + 1; 
                    buf_sent[2] = (sent_Num & 0xFF00) >>8;
                    buf_sent[3] = (sent_Num & 0x00FF);
                    
                    fseek(file_ptr,f_off_set*MAX_DATA_LEN,SEEK_SET);
                    memset(data_sent,0,sizeof(data_sent));
                    
                    read_bytes = fread(data_sent,1,MAX_DATA_LEN,file_ptr);
                    
                    
                    
                    if(read_bytes < 512) 
                    {
                      
                        if(read_bytes == 0)
                        {
                           buf_sent[4] = '\0' ; 
                           printf("ZERO BYTES BLOCK \n");
                        }
                        else
                        {
                           memcpy(&buf_sent[4],data_sent,read_bytes);
                           printf("LESS THAN 512 BYTES : LAST BLOCK READ \n");
                            
                        }
                         block_f_i_n = sent_Num;
                         printf("block_f_i_n : %d \n" ,block_f_i_n);
                                            
                    }
                    else                    
                    {
                        memcpy(&buf_sent[4],data_sent,read_bytes);
                    
                    }
                    
                    if((sent_Bytes_Handler = sendto(child_handle,buf_sent,read_bytes+4,0,(struct sockaddr *)&cli,size_cli)) < 0)
                    {   
                       printf("---error sending\n");
                       if(errno)
                       {
                          printf("----exited coz of error : %s\n", strerror(errno)); 
                       }
                        break;
                    
                    }
                    
                    
                    if(t = t_check(child_handle) == 0)
                    {
                        if(count_of_time_out == 10)
                        {
                            printf("MAXIMUM OF 10 TIME_OUTS  \n");
                            break;
                            
                        }
                        else
                        {
                            printf("TIME_OUT OCCURRED \n");
                            count_of_time_out++;
                            continue;
                        }
                    
                    }
                    else if(t == -1)
                    {
                       printf("---CHILD SELECT ERROR \n");
                       if(errno)
                            printf("Exiting due to error : %s\n", strerror(errno)); 
                        break;
                    }
                    else
                    {
                    
                        memset(r_buf,0,sizeof(r_buf));
                       if((recv_bytes_Handler = recvfrom(child_handle,r_buf,sizeof(r_buf),0,(struct sockaddr*)&cli,&size_cli)) < 0)
                        {
                         printf("CHILD RECEIVE ERROR \n");
                          if(errno)
                            printf("----exited coz of error : %s\n", strerror(errno)); 
                         break;   
                        }
                        
                        op_code = r_buf[1];
                        memcpy(&num_recv, &r_buf[2],2);
                        ack_no = ntohs(num_recv);
                        
                        printf("op_code RECEIVED %d \n " , op_code);
                        printf("ACK RECEIVED : ack_no %d \n",ack_no);
                        if(op_code == 4) 
                        {
                                
                                
                                if(ack_no == block_f_i_n && read_bytes <512)
                                {
                                    printf("FINAL ACK RECEIVED \n");
                                    break;
                                
                                }
                                
                                sent_block_Num=sent_block_Num+1;
                                f_off_set=f_off_set+1;
                        }
                        else
                        {
                            printf("WRONG ACK FORMAT \n");
                            break;
                       
                        }
                    }
                    
            }
          
          if(m_o_d_e ==1)
            {
                if(remove(temp_name_of_file)!=0)
                    printf("ERROR DELETING TEMP FILE \n");
                else
                    printf("TEMP FILE IS DELETED \n");
            }   
            
         close(child_handle);
         printf("CHILD DISCONNECTED \n");
         exit(0); 
            
        }

    }
}


