// NOTE: This file is compiled as Objective-C
#import "AF_Window_Cocoa.h"
#import <Cocoa/Cocoa.h>

// The AppDelegate class conforms to the NSApplicationDelegate and NSWindowDelegate protocols
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property (strong, nonatomic) NSWindow *window; // Property to hold the main application window
@end

// The AppDelegate class implementation
@implementation AppDelegate

// Called when the application is about to finish launching
- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    NSLog(@"Create OSX Window\n");

    // Define the window frame and style
    NSRect frame = NSMakeRect(0, 0, 800, 600);
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    
    // Initialize the window with the defined frame and style
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                               styleMask:style
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];
    
    // Set the window title
    [self.window setTitle:@"AF_Lib OSX"];
    
    // Make the window key and frontmost
    [self.window makeKeyAndOrderFront:nil];
    
    // Set the AppDelegate as the window's delegate
    [self.window setDelegate:self];
    
    // Set the background color to white
    [self.window setBackgroundColor:[NSColor whiteColor]];
}

// Called when the last window is closed; return YES to terminate the application
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    NSLog(@"Shutdown OSX Window\n");
    return YES;
}

@end

// The AF_Window_Cocoa class implementation
@implementation AF_Window_Cocoa

// Class method to create and display the OSX window
+ (void)CreateOSXWindow {
    // Create an instance of AppDelegate
    AppDelegate *delegate = [[AppDelegate alloc] init];
    
    // Get the shared application instance
    NSApplication *app = [NSApplication sharedApplication];
    
    // Set the delegate to handle application events
    [app setDelegate:delegate];
    
    // Run the application event loop
    [app run];
}

@end
