#include "ArchiverException.hpp"

#include <utility>

namespace utar {

    ArchiverException::ArchiverException(std::string error_message) : m_ErrorMessage(std::move(error_message)) {

    }

    const char *ArchiverException::what() const noexcept {
        return m_ErrorMessage.data();
    }

}