#import <Foundation/Foundation.h>

extern int TCCAccessResetForPath(NSString *service, NSString *path);

int main(int argc, char *argv[])
{
        NSString *svc = @"kTCCServiceSystemPolicyAllFiles";
        NSString *path = [NSString stringWithFormat: @"%s", argv[1]];
        int res = TCCAccessResetForPath(svc, path);

        NSLog(@"Arg0: %@, Arg1: %@, Res: %i", svc, path, res);
        return 0;
}

// clang -framework Foundation -framework TCC -F /System/Library/PrivateFrameworks test.m -o test
