#ifndef YAN_DITHER_H
#define YAN_DITHER_H

#include <vector>
#include <filesystem>
#include <climits>
#include <cmath>
#include <algorithm>

#include <yanconv.h>

namespace dither
{
    template<typename T>
    struct color
    {
        T r = 0;
        T g = 0;
        T b = 0;

        color() {};

        color(const T r, const T g, const T b)
        : r(std::clamp(r, static_cast<T>(0), static_cast<T>(255))),
            g(std::clamp(g, static_cast<T>(0), static_cast<T>(255))),
            b(std::clamp(b, static_cast<T>(0), static_cast<T>(255)))
        {
        }

        template<typename P>
        color(const color<P>& c)
        : r(static_cast<T>(c.r)), g(static_cast<T>(c.g)), b(static_cast<T>(c.b))
        {
        }

        template<typename P>
        color<T>& operator*=(const P rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;

            return *this;
        }

        template<typename P>
        friend color<T> operator*(color<T> lhs, const P rhs)
        {
            lhs *= rhs;
            return lhs;
        }

        template<typename P>
        color<T>& operator/=(const P rhs)
        {
            r /= rhs;
            g /= rhs;
            b /= rhs;

            return *this;
        }

        template<typename P>
        friend color<T> operator/(color<T> lhs, const P rhs)
        {
            lhs /= rhs;
            return lhs;
        }

        template<typename P>
        color<T>& operator+=(const color<P>& rhs)
        {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;

            return *this;
        }

        template<typename P>
        friend color<T> operator+(color<T> lhs, const color<P>& rhs)
        {
            lhs += rhs;
            return lhs;
        }

        template<typename P>
        color<T>& operator-=(const color<P>& rhs)
        {
            r -= rhs.r;
            g -= rhs.g;
            b -= rhs.b;

            return *this;
        }

        template<typename P>
        friend color<T> operator-(color<T> lhs, const color<P>& rhs)
        {
            lhs -= rhs;
            return lhs;
        }

        friend std::ostream& operator<<(std::ostream& out, const color<T>& rhs)
        {
            out << "[r: " << rhs.r;
            out << ", g: " << rhs.g;
            out << ", b: " << rhs.b << "]";

            return out;
        }

        template<typename P>
        color<P> cast() const noexcept
        {
            return color<P>{*this};
        }

        int distance(const color<T>& rhs) const noexcept
        {
            return std::abs(r-rhs.r)
                + std::abs(g-rhs.g)
                + std::abs(b-rhs.b);
        }
    };

    struct color_xyz
    {
        float X = 0;
        float Y = 0;
        float Z = 0;

        color_xyz();
        color_xyz(const float X, const float Y, const float Z);
        color_xyz(const color<float>& c);

        int distance(const color_xyz& rhs) const noexcept;
    };

    struct color_lab
    {
        float L = 0;
        float a = 0;
        float b = 0;

        color_lab();
        color_lab(const float L, const float a, const float b);
        color_lab(const color<float>& c);

        friend std::ostream& operator<<(std::ostream& out, const color_lab& rhs);

        float distance(const color_lab& rhs) const noexcept;
    };

    typedef std::vector<color<int>> colors_base;
    class parser
    {
    public:
        static colors_base parse_colors(std::string colors) noexcept;
        static colors_base parse_colors(const std::filesystem::path path) noexcept;
    };

    class ditherer_base
    {
    public:
        enum class dither_type{floyd_steinberg, atkinson, jarvis, ordered};

        ditherer_base();
        ditherer_base(const yconv::image img);
        virtual ~ditherer_base() = default;

        static dither_type parse_type(const std::string str);

        void resize_total(const unsigned total);
        void resize_scale(const float scale_width, const float scale_height);
        void resize(const unsigned width, const unsigned height);

        unsigned width() const noexcept;
        unsigned height() const noexcept;

    protected:
        yconv::image _image;
    };

    template<class T_color>
    class ditherer : public ditherer_base
    {
    public:
        struct distrib_vals
        {
            int multiplier;
            int x;
            int y;
        };

        typedef std::vector<T_color> colors_type;

        ditherer() {};

        ditherer(const yconv::image image, const colors_base colors)
        : ditherer_base(image), _colors_base(colors)
        {
            _colors.reserve(_colors_base.size());
            for(const auto& c : _colors_base)
                _colors.emplace_back(T_color{c});
        }

        yconv::image dither(const dither_type type, const float error_mult = 1) const
        {
            if(_image.bpp!=3 && _image.bpp!=4)
                throw std::runtime_error(std::string("cant dither image with bits per pixel value: ") + std::to_string(_image.bpp));

            switch(type)
            {
                case dither_type::ordered:
                return dither_ordered(std::vector<float>{0.25f, 0.5f, 0.75f, 1.0f}, 2, error_mult);

                case dither_type::floyd_steinberg:
                case dither_type::atkinson:
                case dither_type::jarvis:
                default:
                return dither_generic(type, error_mult);
            }
        }

    private:
        yconv::image dither_ordered(const std::vector<float> pattern, const int p_width, const float error_mult) const
        {
            /*const int p_height = pattern.size()/p_width;

            std::vector<uint8_t> data;

            data.reserve(_image.width*_image.height*_image.bpp);
            for(int y = 0; y < _image.height; ++y)
            {
                for(int x = 0; x < _image.width; ++x)
                {
                    const color<int> c = color<int>{
                        _image.pixel_color(x, y, 0),
                        _image.pixel_color(x, y, 1),
                        _image.pixel_color(x, y, 2)};

                    const color<int> error{0, 0, 0};
                    for(int p = 0; p < pattern.size(); ++p)
                    {
                        const color<int> test_color = c + error;
                        const color<int> closest_color = nearest_color(T_color(test_color));

                    }

                    data.emplace_back();
                    data.emplace_back();
                    data.emplace_back();

                    if(_image.bpp==4)
                    {
                        data.emplace_back(_image.pixel_color(x, y, 3));
                    }
                }
            }

            return yconv::image(_image.width, _image.height, _image.bpp, data);*/
            //TODO
        }

        yconv::image dither_generic(const dither_type type, const float error_mult) const
        {
            const size_t img_size = _image.width*_image.height*_image.bpp;

            std::vector<color<float>> errors(img_size);

            std::vector<uint8_t> data;

            data.reserve(img_size);
            for(int y = 0; y < _image.height; ++y)
            {
                for(int x = 0; x < _image.width; ++x)
                {
                    const color<float> c = (errors[y*_image.width+x]*error_mult)
                        + color<float>{
                        static_cast<float>(_image.pixel_color(x, y, 0)),
                        static_cast<float>(_image.pixel_color(x, y, 1)),
                        static_cast<float>(_image.pixel_color(x, y, 2))};

                        const color<int> out_color = nearest_color(T_color{c});

                        const color<float> error = c-out_color.cast<float>();

                        switch(type)
                        {
                            case dither_type::floyd_steinberg:
                                error_mapped(16, std::vector<distrib_vals>{
                                    {7, 1, 0},
                                    {5, 0, 1},
                                    {3, -1, 1},
                                    {1, 1, 1}},
                                    errors, error, x, y);
                                break;

                            case dither_type::atkinson:
                                error_mapped(8, std::vector<distrib_vals>{
                                    {1, 0, 1},
                                    {1, 0, 2},
                                    {1, 1, 0},
                                    {1, 1, 1},
                                    {1, 2, 0},
                                    {1, -1, 1}},
                                    errors, error, x, y);
                                break;

                            case dither_type::jarvis:
                                error_mapped(48, std::vector<distrib_vals>{
                                    {7, 1, 0},
                                    {5, 2, 0},
                                    {3, -2, 1},
                                    {5, -1, 1},
                                    {7, 0, 1},
                                    {5, 1, 1},
                                    {3, 2, 1},
                                    {1, -2, 2},
                                    {3, -1, 2},
                                    {5, 0, 2},
                                    {3, 1, 2},
                                    {1, 2, 2}},
                                    errors, error, x, y);
                                break;

                            default:
                                throw std::runtime_error("unsupported dither type (how did u do that?)");
                        }

                        data.emplace_back(out_color.r);
                        data.emplace_back(out_color.g);
                        data.emplace_back(out_color.b);

                        if(_image.bpp==4)
                        {
                            data.emplace_back(_image.pixel_color(x, y, 3));
                        }
                }
            }

            return yconv::image(_image.width, _image.height, _image.bpp, data);
        }

        void error_mapped(const float divisor, const std::vector<distrib_vals> distrib_map,
            std::vector<color<float>>& errors, const color<float> error, const int x, const int y) const noexcept
        {
            const color<float> val = error * (1/divisor);

            for(const auto& d_val : distrib_map)
                error_distribute(errors, val*d_val.multiplier, x, y,
                    d_val.x, d_val.y);
        }

        void error_distribute(std::vector<color<float>>& errors, const color<float> error, const int x, const int y, const int x_d, const int y_d) const noexcept
        {
            if(((x_d>0 && x!=_image.width-x_d) || (x_d<0 && x!=std::abs(x_d+1)) || x_d==0) &&
                ((y_d>0 && y!=_image.height-y_d) || (y_d<0 && y!=std::abs(y_d+1)) || y_d==0))
            {
                errors[(y+y_d)*_image.width+(x+x_d)] += error;
            }
        }

        color<int> nearest_color(const T_color c) const noexcept
        {
            color<int> closest_color;
            int closest_distance = INT_MAX;

            auto c_color = _colors.cbegin();
            auto c_color_base = _colors_base.cbegin();

            for(;c_color!=_colors.cend(); ++c_color, ++c_color_base)
            {
                const int c_distance = c_color->distance(c);


                if(c_distance==0)
                    return *c_color_base;

                if(c_distance < closest_distance)
                {
                    closest_color = *c_color_base;
                    closest_distance = c_distance;
                }
            }

            return closest_color;
        }

        colors_type _colors;
        colors_base _colors_base;
    };
};

#endif