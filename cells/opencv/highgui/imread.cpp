#include<fcntl.h>
#include <sys/file.h>

#include <ecto/ecto.hpp>

#include <opencv2/highgui/highgui.hpp>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>

#include "highgui.h"
using ecto::tendrils;

namespace pt = boost::posix_time;
namespace fs = boost::filesystem;
namespace ecto_opencv
{
  struct imread
  {
    typedef imread C;
    static void
    declare_params(tendrils& params)
    {
      params.declare(&C::image_file, "image_file", "The path to the image to read.", "lena.jpg");
      params.declare(&C::mode, "mode", "The image read mode.", Image::COLOR);
      params.declare(&C::lock_name_, "lock_name",
                     "If set to something, an flock will be created for that file", "");
    }

    static void
    declare_io(const tendrils& params, tendrils& inputs, tendrils& outputs)
    {
      //set outputs
      outputs.declare(&C::image_out, "image", "The image in full color.", cv::Mat());
    }
    void
    filechange(const std::string& file_name)
    {
      cv::Mat image;
      if (!lock_name_->empty())
      {
        int file = open(lock_name_->c_str(), O_WRONLY);
        flock(file, LOCK_EX);
        image = cv::imread(file_name, *mode);
        flock(file, LOCK_UN);
        close(file);
      }
      else
        image = cv::imread(file_name, *mode);

      *image_out = image;
      std::cout << "read image:" << file_name << std::endl;
      std::cout << "width:" << image.cols << " height:" << image.rows << " channels:" << image.channels() << std::endl;
    }

    void
    configure(const tendrils& params, const tendrils& inputs, const tendrils& outputs)
    {
      image_file.set_callback(boost::bind(&imread::filechange, this, _1));
      image_file.dirty(true);
    }
    ecto::spore<cv::Mat> image_out;
    ecto::spore<Image::Modes> mode;
    ecto::spore<std::string> image_file;
    ecto::spore<std::string> lock_name_;
  };
}
ECTO_CELL(highgui, ecto_opencv::imread, "imread", "Reads a single image, const cell.");
