#include "F_gauge_fix.h"

/// Compute g * MPS
/// \param g left gauge matrix (only diagonal elements are stored)
/// \param mps MPS
void l_gauge_fix (const std::vector<double>& g, MPS<double>& mps)
{
  for(size_t k = 0; k < mps.matrix_u.trange().tiles().volume(); ++k) {
    if(!mps.matrix_u.is_zero(k) && mps.matrix_u.is_local(k)) {
      auto x = mps.matrix_u.find(k).get();
      size_t lRow = x.range().start()[0];
      size_t uRow = x.range().finish()[0];
      size_t nCols = x.range().size()[1];
      size_t ijOrd = 0;
      for(size_t i = lRow; i < uRow; ++i)
        for(size_t j = 0; j < nCols; ++j, ++ijOrd) x[ijOrd] *= g[i];
    }
  }

  for(size_t k = 0; k < mps.matrix_d.trange().tiles().volume(); ++k) {
    if(!mps.matrix_d.is_zero(k) && mps.matrix_d.is_local(k)) {
      auto x = mps.matrix_d.find(k).get();
      size_t lRow = x.range().start()[0];
      size_t uRow = x.range().finish()[0];
      size_t nCols = x.range().size()[1];
      size_t ijOrd = 0;
      for(size_t i = lRow; i < uRow; ++i)
        for(size_t j = 0; j < nCols; ++j, ++ijOrd) x[ijOrd] *= g[i];
    }
  }
}

void r_gauge_fix (const std::vector<double>& g, MPS<double>& mps)
{
  for(size_t k = 0; k < mps.matrix_u.trange().tiles().volume(); ++k) {
    if(!mps.matrix_u.is_zero(k) && mps.matrix_u.is_local(k)) {
      auto x = mps.matrix_u.find(k).get();
      size_t nRows = x.range().size()[0];
      size_t lCol = x.range().start()[1];
      size_t uCol = x.range().finish()[1];
      size_t ijOrd = 0;
      for(size_t i = 0; i < nRows; ++i)
        for(size_t j = lCol; j < uCol; ++j, ++ijOrd) x[ijOrd] *= g[j];
    }
  }

  for(size_t k = 0; k < mps.matrix_d.trange().tiles().volume(); ++k) {
    if(!mps.matrix_d.is_zero(k) && mps.matrix_d.is_local(k)) {
      auto x = mps.matrix_d.find(k).get();
      size_t nRows = x.range().size()[0];
      size_t lCol = x.range().start()[1];
      size_t uCol = x.range().finish()[1];
      size_t ijOrd = 0;
      for(size_t i = 0; i < nRows; ++i)
        for(size_t j = lCol; j < uCol; ++j, ++ijOrd) x[ijOrd] *= g[j];
    }
  }
}

/// Compute g^-1 * MPS
void l_gauge_fix_inverse (const std::vector<double>& g, MPS<double>& mps)
{
  for(size_t k = 0; k < mps.matrix_u.trange().tiles().volume(); ++k) {
    if(!mps.matrix_u.is_zero(k) && mps.matrix_u.is_local(k)) {
      auto x = mps.matrix_u.find(k).get();
      size_t lRow = x.range().start()[0];
      size_t uRow = x.range().finish()[0];
      size_t nCols = x.range().size()[1];
      size_t ijOrd = 0;
      for(size_t i = lRow; i < uRow; ++i)
        for(size_t j = 0; j < nCols; ++j, ++ijOrd) x[ijOrd] /= g[i];
    }
  }

  for(size_t k = 0; k < mps.matrix_d.trange().tiles().volume(); ++k) {
    if(!mps.matrix_d.is_zero(k) && mps.matrix_d.is_local(k)) {
      auto x = mps.matrix_d.find(k).get();
      size_t lRow = x.range().start()[0];
      size_t uRow = x.range().finish()[0];
      size_t nCols = x.range().size()[1];
      size_t ijOrd = 0;
      for(size_t i = lRow; i < uRow; ++i)
        for(size_t j = 0; j < nCols; ++j, ++ijOrd) x[ijOrd] /= g[i];
    }
  }
}

void r_gauge_fix_inverse (const std::vector<double>& g, MPS<double>& mps)
{
  for(size_t k = 0; k < mps.matrix_u.trange().tiles().volume(); ++k) {
    if(!mps.matrix_u.is_zero(k) && mps.matrix_u.is_local(k)) {
      auto x = mps.matrix_u.find(k).get();
      size_t nRows = x.range().size()[0];
      size_t lCol = x.range().start()[1];
      size_t uCol = x.range().finish()[1];
      size_t ijOrd = 0;
      for(size_t i = 0; i < nRows; ++i)
        for(size_t j = lCol; j < uCol; ++j, ++ijOrd) x[ijOrd] /= g[j];
    }
  }

  for(size_t k = 0; k < mps.matrix_d.trange().tiles().volume(); ++k) {
    if(!mps.matrix_d.is_zero(k) && mps.matrix_d.is_local(k)) {
      auto x = mps.matrix_d.find(k).get();
      size_t nRows = x.range().size()[0];
      size_t lCol = x.range().start()[1];
      size_t uCol = x.range().finish()[1];
      size_t ijOrd = 0;
      for(size_t i = 0; i < nRows; ++i)
        for(size_t j = lCol; j < uCol; ++j, ++ijOrd) x[ijOrd] /= g[j];
    }
  }
}
