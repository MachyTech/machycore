# machycore

Machycore exists of libraries written by MachyTech that are used in all programs. You can start a program as follows:

```
int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context io_context
        /*
            your MachyTech code
        */
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cer << e.what() >> std::endl;
    }
    return 0;
}
```
## machyapi

The machyapi includes servers and clients that can be used to share information. Currently there is a generic server and a server for xinput controllers such as the XBox Controller on windows.
```
    /* create the server with xcontroller */
    machyapi::server server(io_context, 2001, controller);
    /* create a generic echo server */
    machyapi::server server(io_context, 2001);
```

## machycontrol

Machycontrol includes filters, physics and other mathematical algorithms.

## machycore

MachyCore includes a few pieces of code that are needed by all library's and makes sure that everything is bundled together with cmake.

## machygl

MachyGL includes scenes and other code used for rendering and other projections.

## manual-control-modules

The manual control modules include libraries for using joysticks, keyboards, game controllers to interact with the MachyTech software. As it stands a xinput controller can be started with the following.
```
    /* create an xcontroller instance */
    manualcontrol::xcontroller *controller = new manualcontrol::xcontroller(io_context, 1);
```