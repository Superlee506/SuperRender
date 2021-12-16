#include <nori/gui/gui.h>
#include <nori/core/block.h>
#include <nori/filter/gaussianFilter.h>
#include <nori/core/bitmap.h>

using namespace nori;

int main(int argc, char **argv)
{
    google::InitGoogleLogging("SuperRender");
    google::SetStderrLogging(google::GLOG_INFO);
    if (argc != 2) {
        LOG(ERROR) << "Syntax: " << argv[0] << " <showImage.exr>" <<  endl;
        return -1;
    }
    std::string exrName = "";
    filesystem::path path(argv[1]);
    if (path.extension() != "exr")
    {
        LOG(ERROR) << "Please provide the path to a .exr file."
        << path.filename() << "." << path.extension() << endl;
        return -1;
    }
    exrName = argv[1];
   // Bitmap bitmap(exrName);

    PropertyList propList;
    propList.setFloat("radius", 2.0f);
    propList.setFloat("stddev", 0.5f);
    std::unique_ptr<ReconstructionFilter> pFilter = std::make_unique<GaussianFilter>(propList);
    ImageBlock result((Vector2i(1024,1024)), pFilter.get());
    // result.fromBitmap(bitmap);
    std::unique_ptr<Gui>  pGui = std::make_unique<Gui>(result);
    pGui->draw();
    google::ShutdownGoogleLogging();
    return 0;
}

