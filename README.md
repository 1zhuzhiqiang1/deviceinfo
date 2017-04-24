# deviceinfo
ios设备管理demo，包括设备信息，定位，获取文件结构(转JSON)
```
//
//  ZZHomeVC.m
//  deviceinfo
//
//  Created by anyware on 17/4/18.
//  Copyright © 2017年 anyware. All rights reserved.
//
/*
 1. 苹果设备不能获取IMEI唯一码
 */

#import "ZZHomeVC.h"
#include <dlfcn.h>

#import <sys/utsname.h>
#include <sys/socket.h> // Per msqr
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#import <sys/sockio.h>
#import <sys/ioctl.h>
#import <arpa/inet.h>

#import <CoreLocation/CLLocationManager.h>
#import <CoreLocation/CLLocationManagerDelegate.h>

#import <stdlib.h>

#import "ZZFileNode.h"
#import "JSONKit.h"
#import "MJExtension.h"

#import "AppDelegate.h"

@interface ZZHomeVC () <CLLocationManagerDelegate>
{
    CLLocationManager *locationManager;
}

@end

@implementation ZZHomeVC

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    self.view.backgroundColor = [UIColor whiteColor];
    
    [self addBtn];
    
    [self getMobileInfo];//获取设备信息
    [self getLocation];//获取位置信息
    
}

- (void)addBtn{
    // 调用锁屏代码,失败。iOS不支持代码锁屏
    UIButton *btn = [[UIButton alloc] initWithFrame:CGRectMake(100, 100, 200, 50)];
    [btn setEnabled:false];
    [btn setBackgroundColor:[UIColor orangeColor]];
    [btn setTitle:@"锁屏" forState:UIControlStateNormal];
    [btn addTarget:self action:@selector(btnClick) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn];
    // 执行shell脚本
    UIButton *btn_exec_bash = [[UIButton alloc] initWithFrame:CGRectMake(100, 160, 200, 50)];
    [btn_exec_bash setBackgroundColor:[UIColor orangeColor]];
    [btn_exec_bash setTitle:@"获取目录结构" forState:UIControlStateNormal];
    [btn_exec_bash addTarget:self action:@selector(btn_get_file_struture) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:btn_exec_bash];
    
}
-(void)btn_get_file_struture{
    NSString *basePath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
    NSLog(@"base path=%@",basePath);
    [self dir2json:basePath];
}
-(void)dir2json:(NSString*)basepath{
    NSMutableDictionary *dirMap = [NSMutableDictionary dictionary];
    NSFileManager *fm = [NSFileManager defaultManager];
    ZZFileNode *node = [ZZFileNode nodeWithPath:basepath children:[NSMutableArray array] type:@"dir" fileSize:[self getFileSizeByPath:basepath fileManager:fm]];
    [self dir2map:fm path:basepath node:node];
    [dirMap setObject:node forKey:[basepath lastPathComponent]];
    NSDictionary *fileJson = node.mj_keyValues;
    NSData * json_data = [NSJSONSerialization dataWithJSONObject:fileJson options:NSJSONWritingPrettyPrinted error:nil];
    NSString *dirJson = [[NSString alloc] initWithData:json_data encoding:NSUTF8StringEncoding];
    NSLog(@"文件的目录结构是:%@",dirJson);
}
-(void)dir2map:(NSFileManager*)fm path:(NSString*)path node:(ZZFileNode*)node{
    // 如果是目录
    BOOL isDir;
    if(fm != nil && [fm fileExistsAtPath:path isDirectory:&isDir]){
        if(!isDir){
            NSString *fileSize = [self getFileSizeByPath:path fileManager:fm];
            [node.children addObject:[ZZFileNode nodeWithPath:path children:nil type:@"file" fileSize:fileSize]];
        }
    }
    if(fm != nil && [fm fileExistsAtPath:path isDirectory:&isDir]){
        if(isDir){
            for (NSString* fileName in [fm contentsOfDirectoryAtPath:path error:nil]) {
                NSString *subPath = [path stringByAppendingPathComponent:fileName];
                ZZFileNode *subNode = [ZZFileNode nodeWithPath:subPath children:[NSMutableArray array] type:@"dir" fileSize:[self getFileSizeByPath:subPath fileManager:fm]];
                [node.children addObject:subNode];
                [self dir2map:fm path:subPath node:subNode];
            }
        }
    }
    
}
-(NSString*)getFileSizeByPath:(NSString*)path fileManager:(NSFileManager*)fm{
    if(fm != nil && [fm fileExistsAtPath:path isDirectory:false]){
        NSDictionary *dic = [fm attributesOfItemAtPath:path error:nil];
        return [NSString stringWithFormat:@"%llu",[dic fileSize]];
    }
    return @"0";
}
//遍历目录
-(void)showFiles:(NSString *)path;{
    // 1.判断文件还是目录
    NSFileManager * fileManger = [NSFileManager defaultManager];
    BOOL isDir = NO;
    BOOL isExist = [fileManger fileExistsAtPath:path isDirectory:&isDir];
    if (isExist) {
        // 2. 判断是不是目录
        if (isDir) {
            NSArray * dirArray = [fileManger contentsOfDirectoryAtPath:path error:nil];
            NSString * subPath = nil;
            for (NSString * str in dirArray) {
                subPath  = [path stringByAppendingPathComponent:str];
                BOOL issubDir = NO;
                [fileManger fileExistsAtPath:subPath isDirectory:&issubDir];
                if(issubDir){
                    [self showFiles:subPath];
                }
            }
        }else{
            NSLog(@"%@",path);
        }
    }else{
        NSLog(@"你打印的是目录或者不存在");
    }
}
- (void)btnClick{
    NSLog(@"点击了锁频");
    [self lockScreen];
}


- (void)lockScreen{
    char *gsDylib = "/System/Library/PrivateFrameworks/GraphicsServices.framework/GraphicsServices";
    void *handle = dlopen(gsDylib, RTLD_NOW);
    void (*_GSEventLockDevice)() = dlsym(handle, "GSEventLockDevice");
    if (_GSEventLockDevice)  {
        _GSEventLockDevice();
    }
    if (handle) {
        BOOL locked = FALSE;
        void (*_GSEventLockDevice)() = dlsym(handle, "GSEventLockDevice");
        if (_GSEventLockDevice)  {
            _GSEventLockDevice();
            locked = TRUE;
        }
        
        dlclose(handle);
        
        if (!locked) {
            // We should just terminate the app here too...
            [[UIApplication sharedApplication].delegate applicationWillTerminate:[UIApplication sharedApplication]];
            abort(); // That's very nasty!
        }
    }
}


/*
 kCLLocationAccuracyBest; 电池供电的最高精度
 kCLLocationAccuracyNearestTenMeters; 精确到10米
 kCLLocationAccuracyHundredMeters;精确到100米
 kCLLocationAccuracyKilometer;精确到1000米
 kCLLocationAccuracyThreeKilometers;精确到3000米
 */
- (void)getLocation {
    // 1.获取用户的授权状态(iOS7只要使用到定位,就会直接请求授权)
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    if (status == kCLAuthorizationStatusNotDetermined) {
        if ([self.mgr respondsToSelector:@selector(requestAlwaysAuthorization)]) {
            [self.mgr requestAlwaysAuthorization];
        }
    }
}

- (CLLocationManager *)mgr
{
    if (locationManager == nil) {
        locationManager = [[CLLocationManager alloc] init];
        
        // 设置代理,在代理方法中可以拿到用户的位置
        locationManager.delegate = self;
        
        // 设置定位的精度(精度越高越耗电)
        locationManager.desiredAccuracy = kCLLocationAccuracyBestForNavigation;
        
        // 设置当用户移动的时候,重新来定位
        locationManager.distanceFilter = 10.0;
    }
    
    return locationManager;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [[self mgr] startUpdatingLocation];
}

- (void)viewWillDisappear:(BOOL)animated{
    [super viewWillDisappear:animated];
    [[self mgr] stopUpdatingLocation];
}

- (void)getMobileInfo {
    //这个方法后面会列出来
    NSString *deviceName = [self getDeviceName];
    NSLog(@"设备型号-->%@", deviceName);
    
    NSString *iPhoneName = [UIDevice currentDevice].name;
    NSLog(@"iPhone名称-->%@", iPhoneName);
    
    NSString *appVerion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    NSLog(@"app版本号-->%@", appVerion);
    
    CGFloat batteryLevel = [[UIDevice currentDevice] batteryLevel];
    NSLog(@"电池电量-->%f", batteryLevel);
    
    NSString *localizedModel = [UIDevice currentDevice].localizedModel;
    NSLog(@"localizedModel-->%@", localizedModel);
    
    NSString *systemName = [UIDevice currentDevice].systemName;
    NSLog(@"当前系统名称-->%@", systemName);
    
    NSString *systemVersion = [UIDevice currentDevice].systemVersion;
    NSLog(@"当前系统版本号-->%@", systemVersion);
    
    NSString *uuid = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
    NSLog(@"唯一识别码uuid-->%@", uuid);
    
    struct utsname systemInfo;
    uname(&systemInfo);
    NSString *device_model = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    NSLog(@"device_model-->%@", device_model);
    
    // 这个方法后面会单独列出
    NSString *macAddress = [self getMacAddress];
    NSLog(@"macAddress-->%@", macAddress);
    
    // 这个方法后面会单独列出
    //    NSString *deviceIP = [self getDeviceIPAddresses];
    //    NSLog(@"deviceIP-->%@", deviceIP);
}

- (NSString *)getMacAddress {
    int mib[6];
    size_t len;
    char *buf;
    unsigned char *ptr;
    struct if_msghdr *ifm;
    struct sockaddr_dl *sdl;
    
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    
    if ((mib[5] = if_nametoindex("en0")) == 0) {
        printf("Error: if_nametoindex error/n");
        return NULL;
    }
    
    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        printf("Error: sysctl, take 1/n");
        return NULL;
    }
    
    if ((buf = malloc(len)) == NULL) {
        printf("Could not allocate memory. error!/n");
        return NULL;
    }
    
    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        printf("Error: sysctl, take 2");
        return NULL;
    }
    
    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    ptr = (unsigned char *)LLADDR(sdl);
    
    NSString *outstring = [NSString stringWithFormat:@"%02x%02x%02x%02x%02x%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5)];
    free(buf);
    
    return [outstring uppercaseString];
}

// 获取设备型号然后手动转化为对应名称
- (NSString *)getDeviceName
{
    // 需要#import "sys/utsname.h"
#warning 题主呕心沥血总结！！最全面！亲测！全网独此一份！！
    struct utsname systemInfo;
    uname(&systemInfo);
    NSString *deviceString = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    
    if ([deviceString isEqualToString:@"iPhone3,1"])    return @"iPhone 4";
    if ([deviceString isEqualToString:@"iPhone3,2"])    return @"iPhone 4";
    if ([deviceString isEqualToString:@"iPhone3,3"])    return @"iPhone 4";
    if ([deviceString isEqualToString:@"iPhone4,1"])    return @"iPhone 4S";
    if ([deviceString isEqualToString:@"iPhone5,1"])    return @"iPhone 5";
    if ([deviceString isEqualToString:@"iPhone5,2"])    return @"iPhone 5 (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPhone5,3"])    return @"iPhone 5c (GSM)";
    if ([deviceString isEqualToString:@"iPhone5,4"])    return @"iPhone 5c (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPhone6,1"])    return @"iPhone 5s (GSM)";
    if ([deviceString isEqualToString:@"iPhone6,2"])    return @"iPhone 5s (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPhone7,1"])    return @"iPhone 6 Plus";
    if ([deviceString isEqualToString:@"iPhone7,2"])    return @"iPhone 6";
    if ([deviceString isEqualToString:@"iPhone8,1"])    return @"iPhone 6s";
    if ([deviceString isEqualToString:@"iPhone8,2"])    return @"iPhone 6s Plus";
    if ([deviceString isEqualToString:@"iPhone8,4"])    return @"iPhone SE";
    
    if ([deviceString isEqualToString:@"iPod1,1"])      return @"iPod Touch 1G";
    if ([deviceString isEqualToString:@"iPod2,1"])      return @"iPod Touch 2G";
    if ([deviceString isEqualToString:@"iPod3,1"])      return @"iPod Touch 3G";
    if ([deviceString isEqualToString:@"iPod4,1"])      return @"iPod Touch 4G";
    if ([deviceString isEqualToString:@"iPod5,1"])      return @"iPod Touch (5 Gen)";
    
    if ([deviceString isEqualToString:@"iPad1,1"])      return @"iPad";
    if ([deviceString isEqualToString:@"iPad1,2"])      return @"iPad 3G";
    if ([deviceString isEqualToString:@"iPad2,1"])      return @"iPad 2 (WiFi)";
    if ([deviceString isEqualToString:@"iPad2,2"])      return @"iPad 2";
    if ([deviceString isEqualToString:@"iPad2,3"])      return @"iPad 2 (CDMA)";
    if ([deviceString isEqualToString:@"iPad2,4"])      return @"iPad 2";
    if ([deviceString isEqualToString:@"iPad2,5"])      return @"iPad Mini (WiFi)";
    if ([deviceString isEqualToString:@"iPad2,6"])      return @"iPad Mini";
    if ([deviceString isEqualToString:@"iPad2,7"])      return @"iPad Mini (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPad3,1"])      return @"iPad 3 (WiFi)";
    if ([deviceString isEqualToString:@"iPad3,2"])      return @"iPad 3 (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPad3,3"])      return @"iPad 3";
    if ([deviceString isEqualToString:@"iPad3,4"])      return @"iPad 4 (WiFi)";
    if ([deviceString isEqualToString:@"iPad3,5"])      return @"iPad 4";
    if ([deviceString isEqualToString:@"iPad3,6"])      return @"iPad 4 (GSM+CDMA)";
    if ([deviceString isEqualToString:@"iPad4,1"])      return @"iPad Air (WiFi)";
    if ([deviceString isEqualToString:@"iPad4,2"])      return @"iPad Air (Cellular)";
    if ([deviceString isEqualToString:@"iPad4,4"])      return @"iPad Mini 2 (WiFi)";
    if ([deviceString isEqualToString:@"iPad4,5"])      return @"iPad Mini 2 (Cellular)";
    if ([deviceString isEqualToString:@"iPad4,6"])      return @"iPad Mini 2";
    if ([deviceString isEqualToString:@"iPad4,7"])      return @"iPad Mini 3";
    if ([deviceString isEqualToString:@"iPad4,8"])      return @"iPad Mini 3";
    if ([deviceString isEqualToString:@"iPad4,9"])      return @"iPad Mini 3";
    if ([deviceString isEqualToString:@"iPad5,1"])      return @"iPad Mini 4 (WiFi)";
    if ([deviceString isEqualToString:@"iPad5,2"])      return @"iPad Mini 4 (LTE)";
    if ([deviceString isEqualToString:@"iPad5,3"])      return @"iPad Air 2";
    if ([deviceString isEqualToString:@"iPad5,4"])      return @"iPad Air 2";
    if ([deviceString isEqualToString:@"iPad6,3"])      return @"iPad Pro 9.7";
    if ([deviceString isEqualToString:@"iPad6,4"])      return @"iPad Pro 9.7";
    if ([deviceString isEqualToString:@"iPad6,7"])      return @"iPad Pro 12.9";
    if ([deviceString isEqualToString:@"iPad6,8"])      return @"iPad Pro 12.9";
    
    if ([deviceString isEqualToString:@"i386"])         return @"Simulator";
    if ([deviceString isEqualToString:@"x86_64"])       return @"Simulator";
    
    return deviceString;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark CLLocationManagerDelegate
/*
 locations:位置数据数组，最后一个是当前位置
 */
- (void)locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray *)locations{
    CLLocation * currentLocation = [locations lastObject];
    double altitude = currentLocation.altitude;//高度
    double latitude = currentLocation.coordinate.latitude;
    double longitude = currentLocation.coordinate.longitude;
    NSLog(@"维度=%f",latitude);
    NSLog(@"经度=%f",longitude);
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end

```
