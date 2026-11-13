#include "NavigationGridInput.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

int main()
{
    using namespace NavigationGridInput;
    namespace fs = std::filesystem;
    fs::path root = fs::current_path() / ("navigation-input-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directory(root);
    std::string terrain=(root/"terrain").string(), tree=(root/"tree").string(), collision=(root/"collision").string(), nav=(root/"nav").string();
    auto write=[](std::string const& p, void const* data, std::size_t size) { std::ofstream f(p,std::ios::binary|std::ios::trunc);f.write(static_cast<char const*>(data),size); };
    auto empty=[&]{return EmptyGrid(terrain,tree,collision,nav);};
    int failures=0;
    auto expect=[&](bool pass,char const* scenario){if(!pass){std::cerr<<scenario<<'\n';++failures;}};
    expect(!empty(),"Missing collision root must not be treated as empty");
    write(tree,"VMAP_4.3\1",9);
    expect(empty(),"Known tiled root without ADT/collision/nav is empty");
    write(nav,"bad",3);expect(!empty(),"Existing corrupt navigation must reach loader");fs::remove(nav);
    write(collision,"bad",3);expect(!empty(),"Existing corrupt collision must reach loader");fs::remove(collision);
    write(tree,"VMAP_4.3\0",9);expect(!empty(),"Global WMO may supply geometry");
    write(tree,"VMAP_bad\1",9);expect(!empty(),"Invalid root may not hide missing mesh");
    write(tree,"VMAP_4.3\1",9);
    MapHeader map{};std::memcpy(&map.fields[0],"MAPS",4);map.fields[1]=10;map.fields[5]=44;map.fields[6]=16;
    HeightHeader height{};std::memcpy(&height.magic,"MHGT",4);height.flags=1;height.minimum=height.maximum=-500;
    LiquidHeader liquid{};std::memcpy(&liquid.magic,"MLIQ",4);liquid.flags=3;liquid.type=18;
    auto writeMap=[&]{std::ofstream f(terrain,std::ios::binary|std::ios::trunc);f.write(reinterpret_cast<char*>(&map),sizeof(map));f.write(reinterpret_cast<char*>(&height),sizeof(height));if(map.fields[7])f.write(reinterpret_cast<char*>(&liquid),sizeof(liquid));};
    writeMap();expect(empty(),"Default flat tile without liquid has no geometry");
    height.minimum=height.maximum=0;writeMap();expect(!empty(),"Flat playable ground is not proven empty");height.minimum=height.maximum=-500;
    height.flags=0;writeMap();expect(!empty(),"Terrain needs navigation");height.flags=1;
    map.fields[7]=60;map.fields[8]=16;writeMap();expect(empty(),"Uniform dark ocean has no navigation surface");
    liquid.type=1;writeMap();expect(!empty(),"Ordinary water can have navigation");
    liquid.type=18;liquid.flags=2;writeMap();expect(!empty(),"Per-cell liquid data is uncertain");liquid.flags=3;
    map.fields[1]=99;writeMap();expect(!empty(),"Unknown map version must reach loader");map.fields[1]=10;
    write(terrain,"bad",3);expect(!empty(),"Truncated map must reach loader");
    for (auto const& p:{terrain,tree,collision,nav})fs::remove(p);fs::remove(root);
    if(!failures)std::cout<<"Navigation input availability cases passed\n";
    return failures?1:0;
}
