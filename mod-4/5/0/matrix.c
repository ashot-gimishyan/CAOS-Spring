/*
Problem inf-IV-05-0: python/pymatrixdot
Реализуйте Python-модуль matrix с функцией dot, которая выполняет алгебраическое умножение двух квадратных вещественных матриц заданного размера, представленных в виде списка списков.

В случае, если в качестве аргументов переданы матрицы, в которых количество строк или столбцов меньше указанного размера, то оставшаяся часть матриц дополняется нулями. Если больше указанного размера - то лишние строки или столбцы не используются.

Пример использования модуля:

import matrix

A = [[1,2], [3,4]]
B = [[1,2], [3,4]]

R = matrix.dot(2, A, B)
print(R)
На сервер нужно отпарвить только исходный файл, который будет скомпилирован и слинкован с нужными опциями.


*/

//#include </usr/include/python3.10/Python.h>

#include <stdbool.h>
#include <Python.h>

typedef struct {
    double  *data;
    size_t  N;
} Matr_struct;

void matrix_dot(Matr_struct *A, Matr_struct *B_transposed, Matr_struct *out)
{
    size_t N = A->N;
    out->N = N;
    out->data = calloc(N * N, sizeof(*out->data));
    double res = 0;
    double a = 0;
    double b = 0;
    for (size_t i=0; i<N; ++i) {
        for (size_t j=0; j<N; ++j) {
            res = 0;
            for (size_t k=0; k<N; ++k) {
                a = A->data[i*N + k];
                b = B_transposed->data[j*N + k];
                res += a*b;
            }
            out->data[i*N + j] = res;
        }
    }
}

typedef enum {
    NoError = 0,
    ArgumentTypeMismatch,
    RowTypeMismatch,
    ElementTypeMismatch,
} conversion_error_t;

conversion_error_t pyobject_to_matrix(PyObject *lists, size_t shape, Matr_struct *out, bool transposed)
{
    out->N = shape;
    out->data = calloc(shape * shape, sizeof(*out->data));
    conversion_error_t err = NoError;

    if (!PyList_Check(lists)) {
        err = ArgumentTypeMismatch;
        goto fail;
    }
    size_t height = PyList_Size(lists);
    size_t row_width = 0;
    size_t plain_index = 0;
    double elem_value = 0;
    for (size_t i=0; i<height; ++i) {
        PyObject* row = PyList_GetItem(lists, i);
        if (!PyList_Check(row)) {
            err = RowTypeMismatch;
            goto fail;
        }
        row_width = PyList_Size(row);
        for (size_t j=0; j<row_width; ++j) {
            PyObject* elem = PyList_GetItem(row, j);
            if (!PyNumber_Check(elem)) {
                err = ElementTypeMismatch;
                goto fail;
            }
            elem_value = PyFloat_AsDouble(elem);
            plain_index = transposed ? j*shape + i : i*shape + j;
            out->data[plain_index] = elem_value;
        }
    }
    return NoError;
    fail:
    if (out->data)
        free(out->data);
    out->data = NULL;
    out->N = 0;
    return err;
}

PyObject* Matr_structo_pyobject(const Matr_struct *mat)
{
    size_t N = mat->N;
    PyObject *result = PyList_New(N);
    for (size_t i=0; i<N; ++i) {
        PyObject *row = PyList_New(N);
        PyList_SetItem(result, i, row);
        for (size_t j=0; j<N; ++j) {
            PyObject *elem = PyFloat_FromDouble(mat->data[i*N + j]);
            PyList_SetItem(row, j, elem);
        }
    }
    return result;
}

PyObject* dot(PyObject *module, PyObject *args)
{
    Matr_struct A, B, R;
    PyObject *result = NULL;

    memset(&A, 0, sizeof(A));
    memset(&B, 0, sizeof(B));
    memset(&R, 0, sizeof(R));

    if (PyTuple_Size(args) < 3) {
        goto end;
    }

    PyObject *py_shape = PyTuple_GetItem(args, 0);
    PyObject *py_A = PyTuple_GetItem(args, 1);
    PyObject *py_B = PyTuple_GetItem(args, 2);
    if (!PyLong_Check(py_shape)) {
        goto end;
    }
    size_t shape = PyLong_AsSize_t(py_shape);

    if (0!=pyobject_to_matrix(py_A, shape, &A, false)
            || 0!=pyobject_to_matrix(py_B, shape, &B, true))
    {
        goto end;
    }

    matrix_dot(&A, &B, &R);
    result = Matr_structo_pyobject(&R);

    end:
    if (A.data) free(A.data);
    if (B.data) free(B.data);
    if (R.data) free(R.data);
    return result;
}

PyMODINIT_FUNC PyInit_matrix() {
    static PyMethodDef methods[] = {
        { "dot", dot, METH_VARARGS, "Matrix dot on specified shape and two matrices" },
        { NULL, NULL, 0, NULL }
    };

    static PyModuleDef moduleDef = {
        PyModuleDef_HEAD_INIT,
        "matrix",
        "Fast matrix functions",
        -1,
        methods
    };

    return PyModule_Create(&moduleDef);
}
