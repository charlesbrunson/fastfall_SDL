#pragma once

#include "ff/util/math.hpp"

#include <span>

namespace ff {

enum class gpu_usage : u32 {
    StreamDraw  = 0x88E0,
    StreamRead  = 0x88E1,
    StreamCopy  = 0x88E2,
    StaticDraw  = 0x88E4,
    StaticRead  = 0x88E5,
    StaticCopy  = 0x88E6,
    DynamicDraw = 0x88E8,
    DynamicRead = 0x88E9,
    DynamicCopy = 0x88EA,
};

enum class gpu_mem_access : u32 {
    R,
    W,
    RW
};

// dependency injection to support unit testing
namespace detail {
class gpu_interface {
public:
    static u32 gen_buffer();
    static void delete_buffer(u32 t_id);

    static size_t copy_buffer(u32 t_id, std::span<const std::byte> data, i64 t_offset_bytes);

    static void reset_buffer(u32 t_id, i64 t_size_bytes, gpu_usage t_usage);
    static void reset_buffer(u32 t_id, std::span<const std::byte> data, gpu_usage t_usage);

    static void* map_buffer(u32 t_id, gpu_mem_access t_access);
    static void unmap_buffer(u32 t_id);
};
}

template<class Interface = detail::gpu_interface>
class gpu_buffer_base {
public:
    virtual ~gpu_buffer_base() = default;

    inline u32 id() const { return m_id; }
    inline size_t offset_bytes() const { return m_offset_bytes; }
    inline size_t size() const { return m_size; }
    inline size_t size_bytes() const { return m_size * m_elem_size_bytes; };

protected:
    gpu_buffer_base(u32 t_id, size_t t_offset_bytes, size_t t_size, uint32_t t_elem_size)
        : m_id{t_id}, m_offset_bytes{t_offset_bytes}, m_size{t_size}, m_elem_size_bytes{t_elem_size} {
    };

    u32 m_id;

    size_t m_offset_bytes;

    // size in bytes of each elem
    // if T=void, then this is default to 1
    u32 m_elem_size_bytes = 1;
    size_t m_size;
};

template<class T = std::byte, class I = detail::gpu_interface>
class gpu_buffer;

template<class T = std::byte, class I = detail::gpu_interface>
class gpu_map;

// references a subarea of a buffer in the GPU memory
template<class T = std::byte, class I = detail::gpu_interface>
class gpu_span : public gpu_buffer_base<I> {
    using base = gpu_buffer_base<I>;
public:

    gpu_span(uint32_t t_id, size_t t_offset_bytes, size_t t_size)
        : gpu_buffer_base<I>{t_id, t_offset_bytes, t_size, sizeof(T)} {
    }

    // copy to existing buffer
    size_t copy(std::span<const T> elems, size_t t_offset = 0) {
        size_t offset_bytes = t_offset * base::m_elem_size_bytes;
        assert( (offset_bytes >= base::offset_bytes()) || (offset_bytes + elems.size_bytes() <= base::offset_bytes() + base::size_bytes()) );
        return I::copy_buffer(
            base::id(),
            std::as_bytes(elems),
            base::m_offset_bytes + t_offset * base::m_elem_size_bytes);
    }

    // create span into buffer
    template<class R>
    requires ((std::same_as<T, std::byte> || std::same_as<T, R>) && !std::same_as<R, std::byte>)
    gpu_span<R, I> subspan() {
        return gpu_span<R, I>{
            base::id(),
            base::offset_bytes(),
            std::same_as<T, R> ? base::size() : base::size_bytes() / sizeof(R)
        };
    }

    template<class R>
    requires ((std::same_as<T, std::byte> || std::same_as<T, R>) && !std::same_as<R, std::byte>)
    const gpu_span<R, I> subspan() const {
        return {
            base::id(),
            base::offset_bytes(),
            std::same_as<T, R> ? base::size() : base::size_bytes() / sizeof(R)
        };
    }

    template<class R>
    requires ((std::same_as<T, std::byte> || std::same_as<T, R>) && !std::same_as<R, std::byte>)
    gpu_span<R, I> subspan(size_t t_offset_bytes, size_t t_size_elems) {
        return {
            base::id(),
            base::offset_bytes() + t_offset_bytes,
            t_size_elems
        };
    }

    template<class R>
    requires ((std::same_as<T, std::byte> || std::same_as<T, R>) && !std::same_as<R, std::byte>)
    const gpu_span<R, I> subspan(size_t t_offset_bytes, size_t t_size_elems) const {
        return {
            base::id(),
            base::offset_bytes() + t_offset_bytes,
            t_size_elems
        };
    }

    gpu_span<T, I> subspan() {
        return gpu_span<T, I>{
            base::id(),
            base::offset_bytes(),
            base::size()
        };
    }
    const gpu_span<T, I> subspan() const {
        return gpu_span<T, I>{
            base::id(),
            base::offset_bytes(),
            base::size()
        };
    }

    gpu_span<T, I> subspan(size_t t_offset, size_t t_size) {
        return gpu_span<T, I>{
            base::id(),
            base::offset_bytes() + (t_offset * sizeof(T)),
            t_size
        };
    }
    const gpu_span<T, I> subspan(size_t t_offset, size_t t_size) const {
        return gpu_span<T, I>{
            base::id(),
            base::offset_bytes() + (t_offset * sizeof(T)),
            t_size
        };
    }

    gpu_map<T, I> map(gpu_mem_access t_access = gpu_mem_access::W) {
        return gpu_map<T, I>{
            base::id(),
            base::offset_bytes(),
            base::size(),
            t_access
        };
    }
    const gpu_map<T, I> map(gpu_mem_access t_access = gpu_mem_access::W) const {
        return gpu_map<T, I>{
            base::id(),
            base::offset_bytes(),
            base::size(),
            t_access
        };
    }

};

// owns a buffer in the GPU memory
template<class T, class I>
class gpu_buffer : public gpu_span<T, I> {
    using base = gpu_buffer_base<I>;
public:
    gpu_buffer()
    : gpu_span<T, I>{ I::gen_buffer(), 0, 0 }
    {
    }

    explicit gpu_buffer(size_t t_size, gpu_usage t_usage = gpu_usage::StaticDraw)
    : gpu_span<T, I>{ I::gen_buffer(), 0, t_size }
    , m_usage{ t_usage }
    {
        I::reset_buffer(base::id(), t_size * sizeof(T), t_usage);
    }

    explicit gpu_buffer(std::span<const T> data, gpu_usage t_usage = gpu_usage::StaticDraw)
    : gpu_span<T, I>{ I::gen_buffer(), 0, data.size() }
    , m_usage{ t_usage }
    {
        I::reset_buffer(base::id(), std::as_bytes(data), t_usage);
    }

    gpu_buffer<T, I>(const gpu_buffer<T, I>&) = delete;
    gpu_buffer<T, I>& operator=(const gpu_buffer<T, I>&) = delete;

    gpu_buffer<T, I>(gpu_buffer<T, I>&& t_other) noexcept
    : gpu_span<T, I>{ t_other.id(), 0, t_other.size() }
    , m_usage{ t_other.m_usage }
    {
        t_other.m_id = 0;
        t_other.m_offset_bytes = 0;
        t_other.m_elem_size_bytes = 1;
        t_other.m_size = 0;
    }

    gpu_buffer<T, I>& operator=(gpu_buffer<T, I>&& t_other) noexcept {
        base::m_id = t_other.m_id;
        base::m_offset_bytes = t_other.m_offset_bytes;
        base::m_elem_size_bytes = t_other.m_elem_size_bytes;
        base::m_size = t_other.m_size;
        m_usage = t_other.m_usage;

        t_other.m_id = 0;
        t_other.m_offset_bytes = 0;
        t_other.m_elem_size_bytes = 1;
        t_other.m_size = 0;

        return *this;
    }

    ~gpu_buffer() {
        I::delete_buffer(gpu_span<T, I>::id());
    }

    // orphan buffer, new empty buffer with same size && usages
    void reset() {
        I::reset_buffer(
            base::id(),
            base::size_bytes(),
            m_usage);
    }

    // new empty buffer
    void reset(size_t t_new_size, gpu_usage t_usage = gpu_usage::StaticDraw) {
        I::reset_buffer(base::id(), t_new_size, t_usage);
    }

    // new non-empty buffer
    void reset(std::span<const T> data, gpu_usage t_usage = gpu_usage::StaticDraw) {
        I::reset_buffer(base::id(), std::as_bytes(data), t_usage);
    }

private:
    gpu_usage m_usage;
};

template<class T> gpu_buffer(T[]) -> gpu_buffer<T>;
template<class T, size_t E> gpu_buffer(std::span<T, E>) -> gpu_buffer<T>;
template<class T> gpu_buffer(T[], gpu_usage) -> gpu_buffer<T>;
template<class T, size_t E> gpu_buffer(std::span<T, E>, gpu_usage) -> gpu_buffer<T>;

// for reading/writing in the buffer
template<class T, class I>
class gpu_map {
public:
    gpu_map(uint32_t t_id, size_t t_offset_bytes, size_t t_size, gpu_mem_access t_access)
    : m_id{ t_id }
    {
        void* ptr = I::map_buffer(m_id, t_access);
        if (ptr) {
            m_span = std::span{
                reinterpret_cast<T*>((std::byte*)(ptr) + t_offset_bytes),
                reinterpret_cast<T*>((std::byte*)(ptr) + t_offset_bytes + t_size * sizeof(T))
            };
        }
    }
    ~gpu_map() {
        I::unmap_buffer(m_id);
    }

    operator bool() const { return m_span.data() && !m_span.empty(); }

    auto& operator[](size_t ndx) { return m_span[ndx]; }
    auto& operator[](size_t ndx) const { return m_span[ndx]; }

    auto begin() { return m_span.begin(); }
    auto begin() const { return m_span.begin(); }
    auto cbegin() const { return m_span.cbegin(); }
    auto rbegin() { return m_span.rbegin(); }
    auto rbegin() const { return m_span.rbegin(); }
    auto crbegin() const { return m_span.crbegin(); }

    auto end() { return m_span.end(); }
    auto end() const { return m_span.end(); }
    auto cend() const { return m_span.cend(); }
    auto rend() { return m_span.rend(); }
    auto rend() const { return m_span.rend(); }
    auto crend() const { return m_span.crend(); }

private:
    u32 m_id;
    std::span<T> m_span;
};

}