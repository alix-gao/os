
#include<types.h>
#include<pci.h>
#include<io.h>
#include<graph_disp.h>

//out 0xcf8, in 0xcfc
void pci_read_config_dword(unsigned long addr,unsigned long *value)
{
     outl_p(0xCF8,addr);
     inl_p(0xCFC,*value);
}

int pci_disp_pos_x;
int pci_disp_pos_y;
void init_pci_disp_pos()
{
     pci_disp_pos_x=100;
     pci_disp_pos_y=0;
}

//bus,slot,device
void pci_scan_all()
{
     int bus,device,func;
     unsigned long iobase,regvalue;
     int device_num,temp_num;

     device_num=0;
     for(bus=0;bus<0x100;bus++)
     {
         for(device=0;device<32;device++)
         {
             for(func=0;func<8;func++)
             {
                  iobase=0x80000000+bus*0x10000+device*8*0x100+func*0x100;
                  pci_read_config_dword(iobase,&regvalue);
                  if(0xffff!=(bit_16)(regvalue&0x0000ffff))
                  {
                      device_num++;
                      if(15==device_num)
                      {
                          pci_disp_pos_x=100;//pixel
                          pci_disp_pos_y=12*8;//pixel,so multi 8;
                      }
                      temp_num=(int)(regvalue&0x0000ffff);
                      print_int_graph(temp_num,pci_disp_pos_y,pci_disp_pos_x,3);
                      temp_num=(int)((regvalue&0xffff0000)>>16);
                      print_int_graph(temp_num,pci_disp_pos_y+8*6,pci_disp_pos_x,3);
                      pci_disp_pos_x=pci_disp_pos_x+16;
                  }
             }
         }
     }

     print_int_graph(device_num,pci_disp_pos_y,pci_disp_pos_x,3);//my asus machine is 29;
}

//³õÊ¼»¯º¯Êý
void init_pci()
{
     //check_pci_config1();
     print_char_graph('p',200,200,3);
     print_char_graph('c',208,200,3);
     print_char_graph('i',216,200,3);
     print_string_graph("abcdefghijklmnopqrstuvwxyz 1234567890",222,200,3);//630,200,3);
     print_int_graph(12340,200,216,3);

     init_pci_disp_pos();
     pci_scan_all();
}

