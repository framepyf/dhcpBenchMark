
RM 				= rm -rf
FPIC			= -fPIC -c
# target
TARGET 		:= dhcpv6Tool
SRC = $(wildcard ./*.c)
OBJECTS = $(patsubst %.c,%.o,$(notdir $(SRC)))
CFLAGS  =   # -g
LDFLAGS = -lpthread
STATIC  = -static
	
all: $(TARGET)
$(OBJECTS):$(SRC)
	$(CC)  $(FPIC)  $^ $(CFLAGS)
$(TARGET): $(OBJECTS)
	$(CC) $(STATIC) -o $@ $^ $(LDFLAGS)

clean: 
	$(RM) $(TARGET) $(OBJECTS) 


