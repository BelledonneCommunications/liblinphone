//
//  mainView.m
//  linphone
//
//  Created by jehan on 26/09/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "mainView.h"


@implementation mainView


- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
    }
    return self;
}


- (void)drawRect:(CGRect)rect {
	NSString *hello   = @"Hello, World!";
    CGPoint  location = CGPointMake(10, 20);
    UIFont   *font    = [UIFont systemFontOfSize:24];
    [[UIColor whiteColor] set];
    [hello drawAtPoint:location withFont:font];
	// Drawing code
}


- (void)dealloc {
    [super dealloc];
}


@end
