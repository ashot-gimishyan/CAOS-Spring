/*
Problem inf-IV-01-1: posix/dl/cpp-class-loader
Реализуйте механизм динамической загрузки классов C++ по аналогии с Java [https://docs.oracle.com/javase/7/docs/api/java/lang/ClassLoader.html].

Необходимо реализовать функциональность классов ClassLoader и Class из предлагаемого интерфейса.

Базовый каталог для поиска классов определен в переменной окружения CLASSPATH. Имя класса совпадает с каноническим именем файла библиотеки. Полное имя класса может содержать пространства имет C++, разделяемые символами ::. В этом случае необходимо искать файлы в соответствующих подкаталогах (по аналогии с пакетами в Java или Python).

Пример: класс some::package::ClassInPackage будет находиться в библиотеке $CLASSPATH/some/package/ClassInPackage.so.

Для загруженных классов необходимо уметь создавать их экземляры с помощью метода newInstance. При этом гарантируется, что каждый загружаемый класс имеет конструктор по умолчанию. В загружаемых классах могут быть виртуальные методы и виртуальные деструкторы.

Интерфейс который необходимо реализовать (при отправке в систему он описан в файле interfaces.h, который нужно подключить директивой #include):

#include <string>

class AbstractClass
{
    friend class ClassLoader;
public:
    AbstractClass();
    ~AbstractClass();
protected:
    void* newInstanceWithSize(size_t sizeofClass);
    struct ClassImpl* pImpl;
};

template <class T>
class Class
        : public AbstractClass
{
public:
    T* newInstance()
    {
        size_t classSize = sizeof(T);
        void* rawPtr = newInstanceWithSize(classSize);
        return reinterpret_cast<T*>(rawPtr);
    }
};

enum class ClassLoaderError {
    NoError = 0,
    FileNotFound,
    LibraryLoadError,
    NoClassInLibrary
};


class ClassLoader
{
public:
    ClassLoader();
    AbstractClass* loadClass(const std::string &fullyQualifiedName);
    ClassLoaderError lastError() const;
    ~ClassLoader();
private:
    struct ClassLoaderImpl* pImpl;
};
*/
#include <regex>
#include <string>
#include <dlfcn.h>

class AbstractClass
{
    friend class ClassLoader;
public:
    AbstractClass();
    ~AbstractClass();
protected:
    void* newInstanceWithSize(size_t sizeofClass);
    struct ClassImpl* pImpl;
};

template <class T>
class Class : public AbstractClass
{
public:
    T* newInstance()
    {
        size_t classSize = sizeof(T);
        void* rawPtr = newInstanceWithSize(classSize);
        return reinterpret_cast<T*>(rawPtr);
    }
};

enum class ClassLoaderError {
    NoError = 0,
    FileNotFound,
    LibraryLoadError,
    NoClassInLibrary
};


class ClassLoader
{
public:
    ClassLoader();
    AbstractClass* loadClass(const std::string &fullyQualifiedName);
    ClassLoaderError lastError() const;
    ~ClassLoader();
private:
    struct ClassLoaderImpl* pImpl;
};

// ********************************

struct ClassLoaderImpl {
    ClassLoaderError error;
};

struct ClassImpl {
    void* constructor_ptr = nullptr;
};

// ********************************

AbstractClass::AbstractClass()
{
    pImpl = new ClassImpl();
}

AbstractClass::~AbstractClass()
{
    delete pImpl;
}

void* AbstractClass::newInstanceWithSize(size_t sizeofClass)
{
    void* class_ptr = malloc(sizeofClass);
    typedef void (*constructor_t)(void*);
    auto constructor = (constructor_t)pImpl->constructor_ptr;
    constructor(class_ptr);
    return class_ptr;
};

std::string GetName(const std::string& className)
{
    std::string word = "_ZN";
    int i = 0;
    while (i < className.size()) {
        int nm_length = 0;
        if (className[i] != ':') {
            while (className[i] != ':' && i < className.size()) {
                ++nm_length;
                ++i;
            }
            word += std::to_string(nm_length);
            word += className.substr(i - nm_length, nm_length);
        }
        i++;
    }
    word += "C1Ev";
    return word;
}

AbstractClass* ClassLoader::loadClass(const std::string& fullname)
{
    const std::string pathofclass = getenv("CLASSPATH");
    std::string file = std::regex_replace(fullname, std::regex("::"), "/");
    const std::string fpath = pathofclass + "/" + file + ".so";
    auto* abstractClass = new AbstractClass();
    void* lib = dlopen(fpath.c_str(), RTLD_NOW | RTLD_GLOBAL);

    std::string NameConstructor = GetName(fullname);
    void* SymConstructor = dlsym(lib, NameConstructor.c_str());
    if (SymConstructor == nullptr) {
        pImpl->error = ClassLoaderError::NoClassInLibrary;
        return nullptr;
    }

    abstractClass->pImpl->constructor_ptr = SymConstructor;
    return abstractClass;
}

ClassLoader::ClassLoader()
{
    pImpl = new ClassLoaderImpl();
    pImpl->error = ClassLoaderError::NoError;
}

ClassLoaderError ClassLoader::lastError() const
{
    return pImpl->error;
}

ClassLoader::~ClassLoader()
{
    delete pImpl;
}
