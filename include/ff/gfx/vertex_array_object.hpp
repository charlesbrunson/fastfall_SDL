#pragma once

class vertex_array_object {
public:
    vertex_array_object();
    vertex_array_object(const vertex_array_object&) = delete;
    vertex_array_object(vertex_array_object&&) = default;
    vertex_array_object& operator=(const vertex_array_object&) = delete;
    vertex_array_object& operator=(vertex_array_object&&) = default;
    ~vertex_array_object();

    


};