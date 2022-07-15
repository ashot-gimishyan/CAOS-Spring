/*
Problem inf-IV-05-1: python/pyprimes
Реализуйте Python-модуль primes с функцией factor_out, которая выполняет разложение целого числа на простые множители, и возвращает их в виде списка.

В случае, если число уже является простым, то не нужно раскладывать на множители, а вернуть строку Prime!.

Не забывайте обрабатыать ошибочные ситуации!

Пример использования модуля:

Python 3.8.5 (default, Jul 21 2020, 10:48:26)
[Clang 11.0.3 (clang-1103.0.32.62)] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import primes
>>> primes.factor_out(100)
[1, 2, 2, 5, 5]
>>> primes.factor_out(17)
'Prime!'
>>>
На сервер нужно отпарвить только исходный файл, который будет скомпилирован и слинкован с нужными опциями.
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <Python.h>

typedef struct {
	int64_t *list;
	size_t length;
} prime_list;

void primes_factor_out(int64_t num, prime_list *out)
{
	out->list = calloc(num, sizeof(*out->list));
	size_t length = 2;
	(out->list)[0] = 1;
	int64_t i = 2;
	while (i <= num && num > 1) {
		if(num % i == 0) {
			(out->list)[length - 1] = i;
			num /= i;
			length++;
		}
		else {
			i++;
		}
	}
	out->length = length;
}

PyObject* prime_list_to_pyobject(const prime_list *list)
{
    size_t length = list->length;
    PyObject *result;
    if(length == 3) {
        result = PyUnicode_FromString("Prime!");
        return result;
    }
    result = PyList_New(length - 1);
    for(size_t i = 0; i < length - 1; i++) {
        PyObject *each = PyLong_FromLong(list->list[i]);
        PyList_SetItem(result, i, each);
    }
    return result;
}

PyObject* factor_out(PyObject *module, PyObject *args)
{
    prime_list List;
    memset(&List, 0, sizeof(List));

    PyObject *result = NULL;

    if (PyTuple_Size(args) != 1) {
        goto end;
    }

    PyObject *py_number = PyTuple_GetItem(args, 0);
    if (!PyLong_Check(py_number)) {
        goto end;
    }

    int64_t number = PyLong_AsSize_t(py_number);

    primes_factor_out(number, &List);

    result = prime_list_to_pyobject(&List);

    end:
    if (List.list) { free(List.list); }
    return result;
}

PyMODINIT_FUNC PyInit_primes() {
    static PyMethodDef methods[] = {
        { "factor_out", factor_out, METH_VARARGS, "Factor out positive integer" },
        { NULL, NULL, 0, NULL }
    };

    static PyModuleDef moduleDef = {
        PyModuleDef_HEAD_INIT,
        "primes",
        "Functions concerning primes",
        -1,
        methods
    };

    return PyModule_Create(&moduleDef);
}

/*
int main(int argc, char const* argv[])
{
    Py_Initialize();

    PyObject* value = NULL;
    value = PyLong_FromLong(100);
    //value = PyLong_FromLong(17);
    PyObject* arguments = PyTuple_New(1);
    PyTuple_SetItem(arguments, 0, value);

    PyObject* Self;
    PyObject* ResList = factor_out(Self, arguments);

    PyObject_Print(ResList, stdout, 0);
    Py_Finalize();
}
*/
