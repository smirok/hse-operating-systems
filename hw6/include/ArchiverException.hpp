#ifndef LAB_06_ARCHIVEREXCEPTION_HPP
#define LAB_06_ARCHIVEREXCEPTION_HPP

#include <exception>
#include <string>

namespace utar {

    class ArchiverException : public std::exception {
    public:
        explicit ArchiverException(std::string errorMessage);

        const char *what() const noexcept;

    private:
        std::string m_ErrorMessage;
    };

}

#endif //LAB_06_ARCHIVEREXCEPTION_HPP
