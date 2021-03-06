//------------------------------------------------------------------------------
// GB_dup: make a deep copy of a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A, making a deep copy.  Not user-callable; this function does the work
// for user-callable functions GrB_*_dup.

// There is little use for the following feature, but (*Chandle) and A might be
// identical, with GrB_dup (&A, A).  The input matrix A will be lost, and will
// result in a memory leak, unless the user application does the following
// (which is valid and memory-leak free):

//  B = A ;

//  GrB_dup (&A, A) ;

//  GrB_free (&A) ;

//  GrB_free (&B) ;

// A is the new copy and B is the old copy.  Each should be freed when done.

// PARALLEL: a few large memcpy's, can be done in parallel.

#include "GB.h"

GrB_Info GB_dup             // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // handle of output matrix to create
    const GrB_Matrix A,     // input matrix to copy
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A to duplicate", GB0)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    GB_WAIT (A) ;

    // It would also be possible to copy the pending tuples instead.  This
    // might be useful if the input matrix has just a few of them, and then
    // further calls to setElement will be done on the output matrix C.  On the
    // other hand, if A has lots of pending tuples, C will inherit them, and it
    // will double the work needed to assemble both sets of identical tuples.

    // Copying zombies is easy; this code does it already with almost no
    // change (would need to just set the # of zombies in C).

    //--------------------------------------------------------------------------
    // C = A
    //--------------------------------------------------------------------------

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    (*Chandle) = NULL ;

    // [ create C; allocate C->p and do not initialize it
    // C has the exact same hypersparsity as A.
    GrB_Info info ;
    int64_t anz = GB_NNZ (A) ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, A->type, A->vlen, A->vdim, GB_Ap_malloc, A->is_csc,
        GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, A->plen, anz, true,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // copy the contents of A into C
    C->nvec = A->nvec ;
    C->nvec_nonempty = A->nvec_nonempty ;
    memcpy (C->p, A->p, (A->nvec+1) * sizeof (int64_t)) ;   // do parallel
    if (A->is_hyper)
    { 
        memcpy (C->h, A->h, A->nvec * sizeof (int64_t)) ;   // do parallel
    }
    C->magic = GB_MAGIC ;      // C->p and C->h are now initialized ]
    memcpy (C->i, A->i, anz * sizeof (int64_t)) ;   // do parallel
    memcpy (C->x, A->x, anz * A->type->size) ;      // do parallel

    ASSERT_OK (GB_check (C, "C duplicate of A", GB0)) ;

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

