//
//  main.m
//  linphone
//
//  Created by jehan on 25/09/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "linphone.h"



int main(int argc, char *argv[]) {
    
	
	//linphone* phone = [[linphone alloc] init];
	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, nil, nil);
    //[phone release];
	[pool release];
    return retVal;
}

