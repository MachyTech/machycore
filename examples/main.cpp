#include <machyapi.h>
#include <machygl.h>
#include <machycore.h>
#include <machycam.h>

std::vector<machycore::Variables*> *variables;

void create_env(machycore::Environment *env){
    env->appendVariable( new machycore::StdEnvVariable("GLSL_APP_FRAG", "shaders/image.glsl"));
    env->appendVariable( new machycore::StdEnvVariable("GLSL_APP_VERT", "shaders/basic.vert"));
    env->appendVariable( new machycore::StdEnvVariable("SCENE","robotpath"));
    env->appendVariable( new machycore::StdEnvVariable("TCP_CLIENT_IP", "192.168.178.36"));
    env->appendVariable( new machycore::StdEnvVariable("TCP_CLIENT_PORT", "2001"));
    env->appendVariable( new machycore::StdEnvVariable("HTTP_IP", "127.0.0.1"));
    env->appendVariable( new machycore::StdEnvVariable("HTTP_PORT", "3015"));
    env->appendVariable( new machycore::StdEnvVariable("HTTP_ROUTE","/default"));
    env->appendVariable( new machycore::StdEnvVariable("CURL_WEBURL", "http://0.0.0.0:8000/trajectory_100_fpg_out.txt"));
    env->appendVariable( new machycore::StdEnvVariable("SAMPLE_SIZE", "5"));
    env->appendVariable( new machycore::StdEnvVariable("LINEWIDTH", "10"));
    env->appendVariable( new machycore::StdEnvVariable("WINDOW_WIDTH", "800"));
    env->appendVariable( new machycore::StdEnvVariable("WINDOW_HEIGHT", "600"));
    env->appendVariable( new machycore::StdEnvVariable("MAX_THREADS", "12"));
}

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context io_context;

        /* create the main environment */
        machycore::Environment *env = new machycore::Environment();
        create_env(env);
        env->print();

        /* create our thread pool */
        boost::asio::thread_pool pool(std::stoi(env->get(MAX_THREADS)));
        

        /* create texture instance to which we can attach images */
        machycore::texture_data *texture_instance = new machycore::texture_data[2];

        /* create a camera instance and attach a camera to it */
        machycore::camera_data *cam_instance = new machycore::camera_data[1];
        machycam::cam_session cam(io_context, pool, &texture_instance[0], &cam_instance[0], RTSP_STREAM);

        while(texture_instance[0].dirty){}

        /* create two windows to which we can attach openGL contexts */
        printf("width, height = {%d}{%d}\n",
            texture_instance[0].width, texture_instance[0].height);
        
        machygl::Window *win = new machygl::Window(texture_instance[0].width, texture_instance[0].height, "MachyTech");
        
        /* open the scenes and attach them to a window */
        machygl::scene raw_image_scene(win->window[0], pool, io_context, &texture_instance[0], RAW_CAMERA_OUTPUT);
        
        /* for the hogdescriptor we pass the first camera and second texture */
        machyvision::Detector hogdescriptor(io_context, pool, &texture_instance[1],  &cam_instance[0]);
        
        while(texture_instance[1].dirty){}
        printf("width, height = {%d}{%d}\n",
            texture_instance[1].width, texture_instance[1].height);

        win->add_window(texture_instance[1].width, texture_instance[1].height, "Child Window");
        machygl::scene hog_descriptor_scene(win->window[1], pool, io_context, &texture_instance[1], RAW_CAMERA_OUTPUT);

        pool.join();
        io_context.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}