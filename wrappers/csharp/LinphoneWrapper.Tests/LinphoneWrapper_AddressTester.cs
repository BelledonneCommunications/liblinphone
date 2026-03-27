namespace LinphoneWrapper.Tests;

using Linphone;
using System;
using System.Runtime.InteropServices;
using System.IO;

public class LinphoneWrapper_AddressTester
{
    //[DllImport("msvcrt.dll", SetLastError = true)]
    //private static extern IntPtr setlocale(int category, string locale);

    public LinphoneWrapper_AddressTester()
    {
        //IntPtr ptr = setlocale(0, "French_France.1252");
        //if (ptr == IntPtr.Zero)  System.Console.WriteLine("locale could not be set");
    }

    [Theory]
    [InlineData("Marie")]
    [InlineData("Pauline 😊")]
    [InlineData("Laure €€€")]
    [InlineData("Алексей")]
    public void SetUTF8DisplayName(string value)
    {
        Factory factory = Factory.Instance;
        var address = factory.CreateAddress($"\"{value}\" <sip:username@sip.example.com>");

        Assert.NotNull(address);
        Assert.Equal($"{value}", address.DisplayName);
    }
    private void MyOnGlobalStateChanged(Linphone.Core core, Linphone.GlobalState state, string message){
        System.Console.WriteLine("****From MyOnGlobalStateChanged " + message);
    }
    [Fact]
    public void createCore(){
        Factory factory = Factory.Instance;
        factory.DataDir = "./";
        LoggingService ls = LoggingService.Instance;
        ls.LogLevel = LogLevel.Message;
        var core = factory.CreateCore("", "", IntPtr.Zero);
        core.Listener.OnGlobalStateChanged += MyOnGlobalStateChanged;
        core.Start();
        core.Stop();
    }
    [Fact]
    public void testConfiguration(){
        Factory factory = Factory.Instance;
        Config config = factory.CreateConfig("");
        String ret = config.GetString("test", "inexistant-key", "the-default-value");
        Assert.Equal("the-default-value", ret);
    }
}
