#ifndef RDF_SERD_PARSER_HPP
#define RDF_SERD_PARSER_HPP

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>

#include <serd/serd.h>

using namespace std::string_literals;

namespace rdf {
  enum class SerializationFormat {
    NQuads,
    NTriples,
    TriG,
    Turtle
  };
  class SerdParser {
  public:
    SerdParser(const std::filesystem::path& file,
               std::function<void(const std::string& subject, const std::string& predicate, const std::string& object)> callback,
               const std::string& baseIri = "https://example.com/",
               const rdf::SerializationFormat& format = rdf::SerializationFormat::NTriples);
    ~SerdParser() = default;
    static auto on_base(void* handle, const SerdNode* uri) -> SerdStatus;
    static auto on_error(void* handle, const SerdError* error) -> SerdStatus;
    static auto on_prefix(void* handle, const SerdNode* name, const SerdNode* uri) -> SerdStatus;
    static auto on_statement(void* handle,
                             const SerdStatementFlags flags,
                             const SerdNode* graph,
                             const SerdNode* subject,
                             const SerdNode* predicate,
                             const SerdNode* object,
                             const SerdNode* datatypeIri,
                             const SerdNode* languageTag) -> SerdStatus;

  private:
    [[nodiscard]] static auto get_string(const SerdEnv* const environment, const SerdNode* term) -> std::string;
    [[nodiscard]] static auto get_string_object(const SerdEnv* const environment,
                                                const SerdNode* term,
                                                const SerdNode* datatypeIri,
                                                const SerdNode* languageTag) -> std::string;
    [[nodiscard]] auto parser_type(const rdf::SerializationFormat& format) -> SerdSyntax;
    SerdEnv* m_environment = nullptr;
    std::uint64_t m_numByte{0};
    std::function<void(const std::string& subject, const std::string& predicate, const std::string& object)> m_callback;
  };
}

#endif
