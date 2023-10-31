#include <fstream>
#include <string>
#include "../glp/utils.hh"
#include "../glp/model.hh"
#include "../glp/shader.hh"
#include "../glp/sdl.hh"

int main(int argc, char* argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if(argc != 3) {
        fprintf(stderr, "usage: %s [model path] [output file]", argv[0]);
        return 1;
    }

    Window sdl {"glp-util", 1, 1};

    glp_util::Model pb;
    Model model = Model(argv[1], true);
    model.fill_protobuf(&pb);

    std::fstream output(argv[2], std::ios::out | std::ios::trunc | std::ios::binary);
    if(!pb.SerializePartialToOstream(&output)) {
        fprintf(stderr, "%s: failed to write serialize model", argv[0]);
        return 1;
    }

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
