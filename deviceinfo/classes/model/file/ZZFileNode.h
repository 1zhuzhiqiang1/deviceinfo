//
//  ZZFileNode.h
//  deviceinfo
//
//  Created by anyware on 17/4/24.
//  Copyright © 2017年 anyware. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ZZFileNode : NSObject

//"path":"g:\iamsb",
//"children":Array[3],
//"name":"iamsb",
//"type":"dir"

@property (nonatomic,strong) NSString* path;
@property (nonatomic,strong) NSMutableArray* children;
@property (nonatomic,strong) NSString* name;
@property (nonatomic,strong) NSString* type;
@property (nonatomic,strong) NSString* fileSize;

+(ZZFileNode*)nodeWithPath:(NSString*)path children:(NSMutableArray*)children type:(NSString*)type fileSize:(NSString*)fileSize;

@end
