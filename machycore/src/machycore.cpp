#include "machycore.h"

namespace machycore
{
    std::vector<Data> *trajectory = new std::vector<Data>;    
    /* 
     * position vector pointer
     */
    std::vector<Sim> *virposition = new std::vector<Sim>;
    /*
     * virtual position pointer
     */

    std::mutex m_machydata;
    std::condition_variable render_ready;
    bool load_scene = false;
    bool scene_loaded = false;

    void print_pos_data()
    {
        std::cout<<"x, y\n";
        for (const auto &arr: *trajectory){
            std::cout<< arr.x<< ", " << arr.y <<std::endl;
        }
    }

    void print_vpos_data()
    {
        std::cout<<"x, y, t, v, theta"<<std::endl;
        for (const auto &arr: *virposition){
            std::cout<<arr.x<<", "<<arr.y<<", "<<arr.t<<", "<<arr.v<<", "<<arr.theta<<std::endl;
        }
    }

    void print_data_stats()
    {        
        std::cout<<"array size of virtual position : "<<(*virposition).size()<<std::endl;
        std::cout<<"byte size of virtual position : "<<(*virposition).size()*sizeof((*virposition))<<std::endl;
        std::cout<<"pointer to first object : "<<&(*virposition)[0]<<std::endl;
        std::cout<<"byte size of first object : "<<sizeof((*virposition)[0])<<std::endl;
    }
}