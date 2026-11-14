#include "diskrepository.h"
#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <ranges>
#include <lz4frame.h>
#include <snappy.h>
#include <zlib.h>

namespace repository {
    namespace {
        constexpr std::string_view recordPrefix = "HDB1:";
        constexpr char escapeCharacter = '\x1b';

        std::string gzipCompress(const std::string& input) {
            z_stream stream{};
            if(deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                            MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
                throw std::runtime_error("Failed to initialize GZIP compression");
            }

            if(input.size() > std::numeric_limits<uInt>::max()) {
                deflateEnd(&stream);
                throw std::length_error("Input is too large for GZIP compression");
            }

            const uLong bound = deflateBound(&stream, static_cast<uLong>(input.size()));
            if(bound > std::numeric_limits<uInt>::max()) {
                deflateEnd(&stream);
                throw std::length_error("Compressed data is too large for GZIP");
            }

            std::string output(bound, '\0');
            stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data()));
            stream.avail_in = static_cast<uInt>(input.size());
            stream.next_out = reinterpret_cast<Bytef*>(output.data());
            stream.avail_out = static_cast<uInt>(output.size());
            const int result = deflate(&stream, Z_FINISH);
            const uLong outputSize = stream.total_out;
            deflateEnd(&stream);
            if(result != Z_STREAM_END) {
                throw std::runtime_error("GZIP compression failed");
            }

            output.resize(outputSize);
            return output;
        }

        std::string gzipDecompress(const std::string& input) {
            z_stream stream{};
            if(inflateInit2(&stream, MAX_WBITS + 16) != Z_OK) {
                throw std::runtime_error("Failed to initialize GZIP decompression");
            }

            struct InflateContext {
                z_stream* stream;
                ~InflateContext() { inflateEnd(stream); }
            } context{&stream};

            std::string output;
            std::array<char, 16384> buffer{};
            size_t inputOffset = 0;
            int result = Z_OK;

            while(result != Z_STREAM_END) {
                if(stream.avail_in == 0 && inputOffset < input.size()) {
                    const size_t remaining = input.size() - inputOffset;
                    const size_t chunkSize = std::min(remaining, static_cast<size_t>(std::numeric_limits<uInt>::max()));
                    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(input.data() + inputOffset));
                    stream.avail_in = static_cast<uInt>(chunkSize);
                    inputOffset += chunkSize;
                }

                stream.next_out = reinterpret_cast<Bytef*>(buffer.data());
                stream.avail_out = static_cast<uInt>(buffer.size());
                result = inflate(&stream, Z_NO_FLUSH);
                if(result != Z_OK && result != Z_STREAM_END) {
                    throw std::runtime_error("GZIP decompression failed");
                }

                output.append(buffer.data(), buffer.size() - stream.avail_out);
                if(result != Z_STREAM_END && stream.avail_in == 0 && inputOffset == input.size()) {
                    throw std::runtime_error("Truncated GZIP data");
                }
            }

            return output;
        }

        std::string lz4Compress(const std::string& input) {
            const size_t bound = LZ4F_compressFrameBound(input.size(), nullptr);
            if(LZ4F_isError(bound)) {
                throw std::runtime_error(LZ4F_getErrorName(bound));
            }

            std::string output(bound, '\0');
            const size_t outputSize = LZ4F_compressFrame(
                output.data(), output.size(), input.data(), input.size(), nullptr);
            if(LZ4F_isError(outputSize)) {
                throw std::runtime_error(LZ4F_getErrorName(outputSize));
            }

            output.resize(outputSize);
            return output;
        }

        std::string lz4Decompress(const std::string& input) {
            LZ4F_dctx* rawContext = nullptr;
            const size_t createResult = LZ4F_createDecompressionContext(&rawContext, LZ4F_VERSION);
            if(LZ4F_isError(createResult)) {
                throw std::runtime_error(LZ4F_getErrorName(createResult));
            }

            std::unique_ptr<LZ4F_dctx, decltype(&LZ4F_freeDecompressionContext)> context(
                rawContext, LZ4F_freeDecompressionContext);
            std::string output;
            std::array<char, 16384> buffer{};
            size_t inputOffset = 0;
            size_t hint = 1;

            while(hint != 0) {
                size_t inputSize = input.size() - inputOffset;
                size_t outputSize = buffer.size();
                hint = LZ4F_decompress(context.get(), buffer.data(), &outputSize,
                                       input.data() + inputOffset, &inputSize, nullptr);
                if(LZ4F_isError(hint)) {
                    throw std::runtime_error(LZ4F_getErrorName(hint));
                }

                inputOffset += inputSize;
                output.append(buffer.data(), outputSize);
                if(hint != 0 && inputSize == 0 && outputSize == 0) {
                    throw std::runtime_error("Truncated LZ4 data");
                }
            }

            return output;
        }

        std::string compress(const std::string& input, CompressionMethod method) {
            switch(method) {
                case CompressionMethod::Snappy: {
                    std::string output;
                    if(!snappy::Compress(input.data(), input.size(), &output)) {
                        throw std::runtime_error("Snappy compression failed");
                    }
                    return output;
                }
                case CompressionMethod::Gzip:
                    return gzipCompress(input);
                case CompressionMethod::Lz4:
                    return lz4Compress(input);
            }
            throw std::runtime_error("Unknown compression method");
        }

        std::string decompress(const std::string& input, std::string_view method) {
            if(method == "Snappy") {
                std::string output;
                if(!snappy::Uncompress(input.data(), input.size(), &output)) {
                    throw std::runtime_error("Snappy decompression failed");
                }
                return output;
            }
            if(method == "GZIP") return gzipDecompress(input);
            if(method == "LZ4") return lz4Decompress(input);
            throw std::runtime_error("Unknown compression method in persistent record");
        }

        std::string methodName(CompressionMethod method) {
            switch(method) {
                case CompressionMethod::Snappy: return "Snappy";
                case CompressionMethod::Gzip: return "GZIP";
                case CompressionMethod::Lz4: return "LZ4";
            }
            throw std::runtime_error("Unknown compression method");
        }

        std::string escapeRecord(const std::string& input) {
            std::string output;
            output.reserve(input.size());
            for(char character : input) {
                if(character == escapeCharacter) {
                    output += escapeCharacter;
                    output += 'e';
                } else if(character == '\n') {
                    output += escapeCharacter;
                    output += 'n';
                } else {
                    output += character;
                }
            }
            return output;
        }

        std::string unescapeRecord(std::string_view input) {
            std::string output;
            output.reserve(input.size());
            for(size_t i = 0; i < input.size(); ++i) {
                if(input[i] != escapeCharacter) {
                    output += input[i];
                    continue;
                }
                if(++i == input.size()) {
                    throw std::runtime_error("Invalid escape sequence in persistent record");
                }
                if(input[i] == 'e') output += escapeCharacter;
                else if(input[i] == 'n') output += '\n';
                else throw std::runtime_error("Invalid escape sequence in persistent record");
            }
            return output;
        }
    }

    void DiskRepository::loadTo(Repository* const repo) {
        std::ifstream ifs(_filepath, std::ios_base::binary);
        operation::Operation op;

        while(load(ifs, op)) {
            repo->apply(op);
        } 
    }

    void DiskRepository::save(const operation::Operation& op) {
        std::stringstream oss;
        oss << (int)op.type << _separator << op.key;
        for(auto value : op.value) oss << _separator << value.first << _separator << (int)value.second;
        const std::string compressed = compress(oss.str(), _compressionMethod);

        std::ofstream ofs(_filepath, std::ios_base::app | std::ios_base::binary);
        if(!ofs) throw std::runtime_error("Failed to open persistent repository for writing");
        ofs << recordPrefix << methodName(_compressionMethod) << ':' << escapeRecord(compressed) << '\n';
        if(!ofs) throw std::runtime_error("Failed to write persistent repository record");
    }
    
    bool DiskRepository::load(std::ifstream& ifs, operation::Operation& op) {
        std::string line;
        if(!std::getline(ifs, line)) return false;
        
        std::string uncompresed;
        if(line.starts_with(recordPrefix)) {
            const size_t methodEnd = line.find(':', recordPrefix.size());
            if(methodEnd == std::string::npos) {
                throw std::runtime_error("Invalid persistent repository record header");
            }
            const std::string_view method(line.data() + recordPrefix.size(), methodEnd - recordPrefix.size());
            uncompresed = decompress(unescapeRecord(std::string_view(line).substr(methodEnd + 1)), method);
        } else {
            uncompresed = decompress(line, "Snappy");
        }

        auto sp = std::views::split(uncompresed, _separator) | std::ranges::to<std::vector<std::string>>();
        op.type = (operation::OperationType)(std::stoi(sp[0]));
        op.key = sp[1];

        for(int i = 2; i+1 < sp.size(); i += 2) {
            op.value.push_back({sp[i], (types::Type)(std::stoi(sp[i+1]))});
        }

        return true;
    }  
};