# Change these parameters how you want them to be
     CON_PATH = ../concorde-97
           CC ?= mingw32-gcc
       CFLAGS = -O3

# From here nothing needs to be changed
      INCDIRS =  -I$(CON_PATH)

           LD = $(CC)

         LIBS = -lm 

      BLOSSOM = mat_main.c match.c matprice.c 
        PRICE = mp_main.c matprice.c 
     TRI_CALL = tri_call.c           

  BLOSSOMOBJS = $(BLOSSOM:.c=.o)
    PRICEOBJS = $(PRICE:.c=.o)
 TRI_CALLOBJS = $(TRI_CALL:.c=.o)

all    : blossom4 blossom4.a price4 tri_call

blossom4 : $(BLOSSOMOBJS)
	$(LD) -o $@  $(BLOSSOMOBJS) $(CON_PATH)/concorde.a -lm
blossom4.a : $(BLOSSOMOBJS)
	ar ru $@ $^
	ranlib $@
price4 : $(PRICEOBJS)
	$(LD) -o $@  $(PRICEOBJS) $(CON_PATH)/concorde.a -lm
tri_call : $(TRI_CALLOBJS)
	$(LD) -o $@  $(TRI_CALLOBJS) -lm

install : all

clean  :
	rm -f *.o *.exe *.a blossom4 price4 tri_call

depend :
	makedepend $(INCDIRS) $(SEQU)

.c.o :
	$(CC) $(CFLAGS) $(INCDIRS) -c $*.c


