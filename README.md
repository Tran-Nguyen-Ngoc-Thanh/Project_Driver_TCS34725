```c
Phạm Nhựt Tiến - MSSV: 22146416
Bùi Đình Toàn - MSSV: 22146421
Trần Nguyễn Ngọc Thành - MSSV: 22146401
Nguyễn Thị Yến - MSSV: 22146454
```


- [Giới thiệu](#giới-thiệu)
- [Thông số kỹ thuật](#thông-số-kỹ-thuật)
- [Sơ đồ kết nối](#Sơ-đồ-kết-nối)
- [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
  - [Bật chế độ I2C](#bật-chế-độ-I2C)
  - [Kiếm tra địa chỉ I2C của module](#kiểm-tra-địa-chỉ-I2C-của-module)
  - [Cấu hình module](#cấu-hình-module)
  - [Cài đặt](#cài-đặt)
  - [Kiểm tra log kernel và chạy](#kiểm-tra-log-kernel-và-chạy)
  - [Gỡ cài đặt và dọn dẹp](#gỡ-cài-dặt-và-dọn-dẹp)
- [Khuyến nghị khi sử dụng cảm biến](#Khuyến-nghị-khi-sử-dụng-cảm-biến)
- [Tính năng driver](#tính-năng-driver)
- [Tài liệu tham khảo](#tài-liệu-tham-khảo)

# Giới thiệu
Thiết bị TCS34725 cung cấp đầu ra kỹ thuật số cho các giá trị cảm biến ánh sáng đỏ (R), xanh lá (G), xanh dương (B) và ánh sáng trong suốt (clear). Bộ lọc chặn tia hồng ngoại (IR) được tích hợp sẵn trên chip và được định vị tại các đi-ốt quang cảm biến màu, giúp giảm thiểu thành phần phổ IR trong ánh sáng tới và cho phép đo màu chính xác hơn. Với độ nhạy cao, dải động rộng, và bộ lọc chặn IR, TCS34725 là một giải pháp cảm biến màu lý tưởng để sử dụng trong các điều kiện ánh sáng thay đổi và qua các vật liệu làm suy giảm ánh sáng. Ngoài ra, bộ lọc chặn tia IR còn cho phép TCS34725 thực hiện chức năng cảm biến ánh sáng môi trường (ALS - Ambient Light Sensing). Cảm biến ánh sáng môi trường được sử dụng rộng rãi trong các thiết bị có màn hình như điện thoại di động, máy tính xách tay và TV, để cảm nhận điều kiện ánh sáng xung quanh và tự động điều chỉnh độ sáng màn hình nhằm tối ưu hóa trải nghiệm xem và tiết kiệm điện năng. Bản thân TCS34725 có thể chuyển sang trạng thái chờ tiết kiệm năng lượng giữa các lần đo ánh sáng, giúp giảm mức tiêu thụ điện trung bình.
Cảm biến màu TCS34725 có phạm vi ứng dụng rộng, bao gồm:
- Điều khiển đèn nền RGB LED
- Chiếu sáng trạng thái rắn (solid-state lighting)
- Các sản phẩm chăm sóc sức khỏe/thể dục
- Điều khiển quy trình công nghiệp
- Thiết bị chẩn đoán y tế

# Thông số kỹ thuật
- Địa chỉ I2C: 0x29  
- Nguồn cấp: 3.3v - 3.6v  
- Điện áp đầu ra: 3.8  
- Dòng điện đầu ra: 20mA  
- Nhiệt độ môi trường hoạt động: -30 ℃ đến 70 ℃   

# Sơ đồ kết nối
Phần cứng của TCS34725:  
- Pin 1: VDD (Nguồn cấp)  
- Pin 2: SCL (Chân đầu vào xung đồng hồ I²C dùng để đồng bộ tín hiệu dữ liệu I²C)  
- Pin 3: GND (Nối đất, tất cả điện áp đều được tham chiếu đến GND)  
- Pin 4: NC  (không kết nối)  
- Pin 5: INT (Ngắt (tín hiệu ngắt đầu ra dạng open-drain, kích hoạt mức thấp))  
- Pin 6: SDA (Dữ liệu I²C (chân nhập/xuất dữ liệu nối tiếp qua giao thức I²C))  

Sơ đồ kết nối TCS34725 với rapsberry:    
- VCC -> 1(3,3V)  
- GND -> 6  
- SCL -> 5  
- SDA -> 3  

# Hướng dẫn sử dụng
### Bật chế độ I2C 

```c
sudo raspi-config
```
Sau đó chọn: Interface option -> I2C -> Enable

### Kiếm tra địa chỉ I2C của module
```c
sudo i2cdetect -y 1
```
-	Nếu cắm TCS34725 vào như phần cứng ở trên mà xuất hiện địa chỉ 0x29, kết nối thành công.  
-	Nếu hiển thị địa chỉ UU tại vị trí 0x29, lỗi này có thể bị chiếm bởi 1 driver khác.

### Cấu hình module
Bước 1:  
(raspberry pi 4 và pi 5)  
```c
cd /boot/firmware
``` 
(raspberry pi 3)  
```c
cd /boot
```  
  
Bước 2:  
Tìm đến file có đầu là bcm2710 (pi3), bcm2711 (pi4), bcm2712 (pi5) và đuôi là .dtb
  
Bước 3:  
Chuyển đổi file bạn vừa kiểm ở trên từ .dtb sang .dts  
(ví dụ là Pi5-b, thay tên file bạn kiếm được, tùy loại Pi)  
```c
dtc -I dtb -O dts -o bcm2712-rpi-5-b.dts -o bcm2712-rpi-5-b.dtb
```  
  
Bước 4:  
Truy cập vào file .dts vừa tạo.  
(ví dụ là Pi5-b, thay tên file bạn kiếm được, tùy loại Pi)  
```c
sudo nano bcm2712-rpi-5-b.dts
```  
  
Bước 5:  
Tìm đến:   
```c
i2c@74000
```  
và nhập vào đoạn sau:  
```c
tcs34725@29{ 
    compatible = "amsco,tcs34725"; 
    reg = <0x29>; 
};
```  
  
Bước 6:  
Chuyển đổi file bạn vừa kiểm ở trên từ .dts sang .dtb  
(ví dụ là Pi5-b, thay tên file bạn kiếm được, tùy loại Pi)  
```c
dtc -I dts -O dtb -o bcm2712-rpi-5-b.dts bcm2712-rpi-5-b.dtb
```  
  
### Cài đặt  
Truy cập vào folder lưu các file driver.  
```c
make
```  
Sau đó:
```c
sudo insmod TCS34725_driver.ko
```  
  
### Kiểm tra log kernel và chạy  
Kiểm tra:  
```c
dmesg | tail
```
Sau đó chạy:
```c
gcc TCS34725_ioctl.c -o run
sudo ./run
```  
  
### Gỡ cài đặt và dọn dẹp file  
```c
sudo rmmod TCS34725_driver
make clean
```  

# Khuyến nghị khi sử dụng cảm biến  
- Ánh sáng môi trường:  
 Tránh ánh sáng quá mạnh hoặc thay đổi đột ngột trong quá trình đo, vì TCS34725 có độ nhạy cao với ánh sáng môi trường.  
 Nếu cần thiết, sử dụng tấm chắn hoặc lọc quang để ổn định ánh sáng khi đo.  
- Vị trí lắp đặt:  
 Đặt cảm biến ở khoảng cách thích hợp từ bề mặt cần đo, tránh quá gần hoặc quá xa để giảm sai số, khuyến khích từ 1-2cm. 
- Bảo vệ khỏi bụi và độ ẩm: 
 Bụi hoặc hơi nước có thể ảnh hưởng đến độ chính xác. Sử dụng nắp bảo vệ trong môi trường bụi bẩn hoặc ẩm ướt.
- Điện áp cung cấp ổn định:  
 Đảm bảo điện áp cấp cho cảm biến nằm trong khoảng khuyến nghị (2.7V – 3.6V), tránh dao động lớn để không gây lỗi trong quá trình đo. 
- Thời gian lấy mẫu (Integration Time):  
 Điều chỉnh thời gian lấy mẫu (Integration Time) phù hợp với điều kiện ánh sáng để đạt được độ chính xác tối ưu mà không bị bão hòa tín hiệu. 
- Nhiệt độ môi trường:  
 Tránh đặt cảm biến gần nguồn nhiệt, vì sự thay đổi nhiệt độ lớn có thể ảnh hưởng đến độ nhạy và độ chính xác của phép đo.  
- Tránh rung lắc:  
 Cảm biến cần được gắn cố định, tránh rung lắc trong quá trình đo để giữ tính ổn định của dữ liệu thu thập.  
- Kiểm tra định kỳ:  
 Định kỳ kiểm tra cảm biến để phát hiện hư hỏng hoặc sai lệch nhằm duy trì hiệu suất tốt nhất.
- Chỉnh độ lợi:  
 Độ lợi cần chỉnh tùy theo ứng dụng phù hợp, ứng dụng cho bề mặt bóng không chính xác  

# Tính năng driver 
Để dùng các chức năng của driver khi người dùng code ở lớp user thì cần thêm thư viện sau:    
```c
#include <sys/ioctl.h>
```  
Và define các chức năng muốn dùng của driver như sau:
- Đọc giá trị kênh Clear
```c
#define TCS34725_IOCTL_READ_C _IOR(TCS34725_IOCTL_MAGIC, 1, int)
```  

- Đọc giá trị kênh RED
 ```c
#define TCS34725_IOCTL_READ_R _IOR(TCS34725_IOCTL_MAGIC, 2, int) 
```  

- Đọc giá trị kênh GREEN
```c
#define TCS34725_IOCTL_READ_G _IOR(TCS34725_IOCTL_MAGIC, 3, int)
```  

- Đọc giá trị kênh BLUE
```c
#define TCS34725_IOCTL_READ_B _IOR(TCS34725_IOCTL_MAGIC, 4, int) 
```  

- Ghi giá trị vào các thanh ghi của TCS34725
```c
#define TCS34725_IOCTL_INIT_IOW(TCS34725_IOCTL_MAGIC, 5, struct tcs34725_config)
```  

- Giao tiếp giữa user với kernel
```c
int ioctl(int fd, unsigned long request, ... /* arg */ );
```  
Các tham số:
1. int fd (File Descriptor):
- Đây là tham số đầu tiên và là một số nguyên đại diện cho file descriptor của thiết bị mà bạn muốn tương tác.
-	ioctl() được gọi trên một file descriptor đã được mở, cho phép bạn gửi các lệnh điều khiển cụ thể đến driver liên kết với thiết bị đó.
2. Unsigned long request:   
-  Đây là tham số để nhập chức năng muốn dùng của drive.
3. /* arg */  
-  Đây là biến con trỏ để lưu giá trị từ kernel gửi lên user hoặc gửi giá trị từ user xuống kernel.


# Tài liệu tham khảo
- [Datasheet TCS34725](https://look.ams-osram.com/m/7ec5bcc3e40679be/original/TCS3472-DS000390.pdf)

