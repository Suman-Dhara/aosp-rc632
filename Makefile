
#call from kernel buiuld system

obj-m	:=	rc632_driver.o rc632.o
 
rc632_driver-y 	:= 	poller/input-polldev.o indriver/rc632_input.o 
 
ccflags-y := -Iinclude
ccflags-y += -Ipoller/include
ccflags-y += -Iindriver/include
