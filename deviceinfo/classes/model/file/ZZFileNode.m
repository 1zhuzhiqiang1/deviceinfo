//
//  ZZFileNode.m
//  deviceinfo
//
//  Created by anyware on 17/4/24.
//  Copyright © 2017年 anyware. All rights reserved.
//

#import "ZZFileNode.h"
#import "MJExtension.h"

@implementation ZZFileNode

+(ZZFileNode*)nodeWithPath:(NSString*)path children:(NSMutableArray*)children type:(NSString*)type fileSize:(NSString*)fileSize{
    ZZFileNode *node = [[ZZFileNode alloc] init];
    node.path = path;
    if(children == nil){
        node.children = [NSMutableArray array];
    }else{
        node.children = children;
    }
    node.name = [path lastPathComponent];
    node.type = type;
    node.fileSize = fileSize;
    return node;
}

MJCodingImplementation
@end
