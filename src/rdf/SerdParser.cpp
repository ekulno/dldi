#include <cstdint>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>

#include <serd/serd.h>

#include "LibzSerdStream.hpp"
#include "SerdParser.hpp"

namespace rdf {
  SerdParser::SerdParser(const std::filesystem::path& file,
                         std::function<void(const std::string& subject, const std::string& predicate, const std::string& object)> callback,
                         const std::string& baseIri,
                         const rdf::SerializationFormat& format)
    : m_callback{callback}, m_numByte{std::filesystem::file_size(file)} {
    // Create base IRI and environment.
    SerdURI base_uri{SERD_URI_NULL};
    SerdNode base{serd_node_new_uri_from_string(reinterpret_cast<const std::uint8_t*>(baseIri.c_str()), nullptr, &base_uri)};
    m_environment = serd_env_new(&base);
    SerdReader* reader = serd_reader_new(parser_type(format),
                                         this,
                                         nullptr,
                                         reinterpret_cast<SerdBaseSink>(on_base),
                                         reinterpret_cast<SerdPrefixSink>(on_prefix),
                                         reinterpret_cast<SerdStatementSink>(on_statement),
                                         nullptr);
    serd_reader_set_error_sink(reader, on_error, nullptr);
    std::uint8_t* const in = serd_file_uri_parse(reinterpret_cast<const std::uint8_t*>(file.c_str()), nullptr);
    if (file.extension() == ".gz") {
      LibzSerdStream libzSerdStream{file};
      const SerdStatus status{serd_reader_read_source(reader,
                                                      &LibzSerdStream::read,
                                                      &LibzSerdStream::error,
                                                      &libzSerdStream,
                                                      in,
                                                      4'096)}; //SERD_PAGE_SIZE
      if (status) {
        throw std::runtime_error(reinterpret_cast<const char*>(serd_strerror(status)));
      }
    } else {
      FILE* in_fd = fopen(reinterpret_cast<const char*>(in), "r");
      // TODO: fadvise sequential
      if (!in_fd) {
        throw std::runtime_error("Could not open input file for parsing.");
      }
      serd_reader_read_file_handle(reader,
                                   in_fd,
                                   reinterpret_cast<const std::uint8_t*>(file.c_str()));
      fclose(in_fd);
    }
    serd_free(in);
    serd_reader_free(reader);
    serd_env_free(m_environment);
    serd_node_free(&base);
    return;
  }

  auto SerdParser::get_string(const SerdEnv* const environment,
                              const SerdNode* const term) -> std::string {
    std::string out;
    out.reserve(term->n_bytes + 2);
    switch (term->type) {
    case SERD_BLANK:
      {
        out.append("_:");
        out.append(reinterpret_cast<const char*>(term->buf), term->n_bytes);
        auto iri{serd_env_expand_node(environment, term)};
        out.append(reinterpret_cast<const char*>(iri.buf), iri.n_bytes);
        serd_node_free(&iri);
        break;
      }
    case SERD_CURIE:
    case SERD_URI:
      {
        auto iri{serd_env_expand_node(environment, term)};
        out.append(reinterpret_cast<const char*>(iri.buf), iri.n_bytes);
        serd_node_free(&iri);
        break;
      }
    default:
      return out;
    }
    // Already handled by switch default, but some compilers emit a warning.
    return out;
  }

  auto SerdParser::get_string_object(const SerdEnv* environment,
                                     const SerdNode* const term,
                                     const SerdNode* const datatypeIri,
                                     const SerdNode* const languageTag) -> std::string {
    if (term->type != SERD_LITERAL) {
      return get_string(environment, term);
    }
    std::string out;
    out.reserve(term->n_bytes + 2 +
                (datatypeIri ? datatypeIri->n_bytes + 4 : 0) +
                (languageTag ? languageTag->n_bytes + 1 : 0));
    out.push_back('\"');
    out.append(reinterpret_cast<const char*>(term->buf), term->n_bytes);
    out.push_back('\"');
    if (languageTag) {
      out.push_back('@');
      out.append(reinterpret_cast<const char*>(languageTag->buf), languageTag->n_bytes);
    }
    if (datatypeIri) {
      out.append("^^<");
      out.append(get_string(environment, datatypeIri));
      out.push_back('>');
    }
    return out;
  }

  auto SerdParser::on_error([[maybe_unused]] void* const handle,
                            const SerdError* const error) -> SerdStatus {
    std::fprintf(stderr,
                 "error: %s:%u:%u: ",
                 error->filename,
                 error->line,
                 error->col);
    throw std::runtime_error("Error parsing input.");
    return error->status;
  }

  // Callback for base URI changes (@base directives).
  auto SerdParser::on_base(void* const handle,
                           const SerdNode* const uri) -> SerdStatus {
    SerdParser* const parser = reinterpret_cast<SerdParser*>(handle);
    return serd_env_set_base_uri(parser->m_environment, uri);
  }

  // Callback for namespace definitions (@prefix directives).
  auto SerdParser::on_prefix(void* const handle,
                             const SerdNode* const name,
                             const SerdNode* const uri) -> SerdStatus {
    SerdParser* const parser = reinterpret_cast<SerdParser*>(handle);
    return serd_env_set_prefix(parser->m_environment, name, uri);
  }

  // Callback for statements.
  auto SerdParser::on_statement(void* const handle,
                                [[maybe_unused]] const SerdStatementFlags flags,
                                [[maybe_unused]] const SerdNode* const graph,
                                const SerdNode* const subject,
                                const SerdNode* const predicate,
                                const SerdNode* const object,
                                const SerdNode* const datatypeIri,
                                const SerdNode* const languageTag) -> SerdStatus {
    const SerdParser* const parser = reinterpret_cast<SerdParser*>(handle);
    parser->m_callback(
      parser->get_string(parser->m_environment, subject),
      parser->get_string(parser->m_environment, predicate),
      parser->get_string_object(parser->m_environment, object, datatypeIri, languageTag));
    return SERD_SUCCESS;
  }

  auto SerdParser::parser_type(const rdf::SerializationFormat& format) -> SerdSyntax {
    switch (format) {
    case rdf::SerializationFormat::NQuads:
      return SERD_NQUADS;
    case rdf::SerializationFormat::NTriples:
      return SERD_NTRIPLES;
    case rdf::SerializationFormat::TriG:
      return SERD_TRIG;
    case rdf::SerializationFormat::Turtle:
      return SERD_TURTLE;
    default:
      throw std::runtime_error("Serd parser only supports N-Triples, N-Quads, TriG, and Turtle.");
    }
  }
}
