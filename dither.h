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
        : r(r), g(g), b(b)
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

    class parser
    {
    public:
        typedef std::vector<color<int>> colors_type;

        static colors_type parse_colors(const std::string colors) noexcept;
    };

    class ditherer_base
    {
    public:
        enum class dither_type{floyd_steinberg, atkinson, jarvis};

        ditherer_base();
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
        typedef std::vector<T_color> colors_type;

        ditherer() {};

        ditherer(const std::filesystem::path image_path, const std::string colors)
        : _colors_base(parser::parse_colors(colors))
        {
            if(!std::filesystem::exists(image_path))
                throw std::runtime_error(std::string("file not found: ")+image_path.string());

            if(yconv::image::can_parse(image_path.extension().string()))
            {
                _image = yconv::image(image_path);
            } else
            {
                if(image_path.extension()==".jpeg" || image_path.extension()==".jpg")
                {
                    //TODO THIS
                } else
                {
                    throw std::runtime_error(std::string("cant parse: ")+image_path.string());
                }
            }

            _colors.reserve(_colors_base.size());
            for(const auto& c : _colors_base)
                _colors.emplace_back(T_color{c});
        }

        yconv::image dither(const dither_type type) const
        {
            if(_image.bpp!=3 && _image.bpp!=4)
                throw std::runtime_error(std::string("cant dither image with bits per pixel value: ") + std::to_string(_image.bpp));

            const size_t img_size = _image.width*_image.height*_image.bpp;

            std::vector<color<float>> errors(img_size);

            std::vector<uint8_t> data;

            data.reserve(img_size);
            for(int y = 0; y < _image.height; ++y)
            {
                for(int x = 0; x < _image.width; ++x)
                {
                    const color<float> c = errors[y*_image.width+x] + color<float>{
                        static_cast<float>(_image.pixel_color(x, y, 0)),
                        static_cast<float>(_image.pixel_color(x, y, 1)),
                        static_cast<float>(_image.pixel_color(x, y, 2))};

                    const color<int> out_color = nearest_color(T_color{c});

                    const color<float> error = c-out_color.cast<float>();

                    switch(type)
                    {
                        case dither_type::floyd_steinberg:
                            error_floyd_steinberg(errors, error, x, y);
                        break;

                        case dither_type::atkinson:
                            error_atkinson(errors, error, x, y);
                        break;

                        case dither_type::jarvis:
                            error_jarvis(errors, error, x, y);
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

    private:
        void error_floyd_steinberg(std::vector<color<float>>& errors, const color<float> error, const int x, const int y) const noexcept
        {
            const color<float> val = error * (1/16.0f);

            error_distribute(errors, val*7, x, y, 1, 0);
            error_distribute(errors, val*3, x, y, -1, 1);
            error_distribute(errors, val*5, x, y, 0, 1);
            error_distribute(errors, val, x, y, 1, 1);
        }

        void error_atkinson(std::vector<color<float>>& errors, const color<float> error, const int x, const int y) const noexcept
        {
            const color<float> val = error * (1/8.0f);

            error_distribute(errors, val, x, y, 1, 0);
            error_distribute(errors, val, x, y, 2, 0);
            error_distribute(errors, val, x, y, -1, 1);
            error_distribute(errors, val, x, y, 0, 1);
            error_distribute(errors, val, x, y, 1, 1);
            error_distribute(errors, val, x, y, 0, 2);
        }

        void error_jarvis(std::vector<color<float>>& errors, const color<float> error, const int x, const int y) const noexcept
        {
            const color<float> val = error * (1/48.0f);

            error_distribute(errors, val*7, x, y, 1, 0);
            error_distribute(errors, val*5, x, y, 2, 0);
            error_distribute(errors, val*3, x, y, -2, 1);
            error_distribute(errors, val*5, x, y, -1, 1);
            error_distribute(errors, val*7, x, y, 0, 1);
            error_distribute(errors, val*5, x, y, 1, 1);
            error_distribute(errors, val*3, x, y, 2, 1);
            error_distribute(errors, val, x, y, -2, 2);
            error_distribute(errors, val*3, x, y, -1, 2);
            error_distribute(errors, val*5, x, y, 0, 2);
            error_distribute(errors, val*3, x, y, 1, 2);
            error_distribute(errors, val, x, y, 2, 2);
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
        std::vector<color<int>> _colors_base;
};
};

#endif