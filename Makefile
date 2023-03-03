
RM 				= rm -rf
FPIC			= -fPIC -c
# target
TARGET 		:= dhcpv6Benchmark
SRC = $(wildcard ./*.c)
SRC += $(wildcard ./dhcp/*.c)
SRC += $(wildcard ./thread/*.c)
SRC += $(wildcard ./net/*.c)
SRC += $(wildcard ./async/*.c)
SRC += $(wildcard ./util/*.c)
SRC += $(wildcard ./log/*.c)
OBJECTS = $(patsubst %.c,%.o,$(notdir $(SRC)))
CFLAGS  =  -I./  -g
LDFLAGS = -lpthread
STATIC  = -static
	
all: $(TARGET)
$(OBJECTS):$(SRC)
	$(CC)  $(FPIC)  $^ $(CFLAGS)
$(TARGET): $(OBJECTS)
	$(CC) $(STATIC) -o $@ $^ $(LDFLAGS)
	$(RM) $(OBJECTS) 

clean: 
	$(RM) $(TARGET) $(OBJECTS) 


