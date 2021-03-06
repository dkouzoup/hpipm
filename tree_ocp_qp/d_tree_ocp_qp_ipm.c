/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High-Performance Interior Point Method.                                                *
* Copyright (C) 2017-2018 by Gianluca Frison.                                                     *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* This program is free software: you can redistribute it and/or modify                            *
* it under the terms of the GNU General Public License as published by                            *
* the Free Software Foundation, either version 3 of the License, or                               *
* (at your option) any later version                                                              *.
*                                                                                                 *
* This program is distributed in the hope that it will be useful,                                 *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                   *
* GNU General Public License for more details.                                                    *
*                                                                                                 *
* You should have received a copy of the GNU General Public License                               *
* along with this program.  If not, see <https://www.gnu.org/licenses/>.                          *
*                                                                                                 *
* The authors designate this particular file as subject to the "Classpath" exception              *
* as provided by the authors in the LICENSE file that accompained this code.                      *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifdef USE_C99_MATH
#include <math.h>
#endif

#include <blasfeo_target.h>
#include <blasfeo_common.h>
#include <blasfeo_d_aux.h>
#include <blasfeo_d_blas.h>

#include <hpipm_d_tree_ocp_qp.h>
#include <hpipm_d_tree_ocp_qp_sol.h>
#include <hpipm_d_tree_ocp_qp_res.h>
#include <hpipm_d_tree_ocp_qp_ipm.h>
#include <hpipm_d_tree_ocp_qp_kkt.h>
#include <hpipm_d_core_qp_ipm.h>
#include <hpipm_d_core_qp_ipm_aux.h>



#define AXPY blasfeo_daxpy
#define BACKUP_RES_M d_backup_res_m
#define COMPUTE_ALPHA_QP d_compute_alpha_qp
#define COMPUTE_CENTERING_CORRECTION_QP d_compute_centering_correction_qp
#define COMPUTE_CENTERING_QP d_compute_centering_qp
#define COMPUTE_LIN_RES_TREE_OCP_QP d_compute_lin_res_tree_ocp_qp
#define COMPUTE_MU_AFF_QP d_compute_mu_aff_qp
#define COMPUTE_RES_TREE_OCP_QP d_compute_res_tree_ocp_qp
#define CORE_QP_IPM_WORKSPACE d_core_qp_ipm_workspace
#define CREATE_TREE_OCP_QP_RES d_create_tree_ocp_qp_res
#define CREATE_TREE_OCP_QP_SOL d_create_tree_ocp_qp_sol
#define CREATE_STRMAT blasfeo_create_dmat
#define CREATE_STRVEC blasfeo_create_dvec
#define CREATE_CORE_QP_IPM d_create_core_qp_ipm
#define FACT_LQ_SOLVE_KKT_STEP_TREE_OCP_QP d_fact_lq_solve_kkt_step_tree_ocp_qp
#define FACT_SOLVE_KKT_STEP_TREE_OCP_QP d_fact_solve_kkt_step_tree_ocp_qp
#define FACT_SOLVE_KKT_UNCONSTR_TREE_OCP_QP d_fact_solve_kkt_unconstr_tree_ocp_qp
#define GELQF_WORKSIZE blasfeo_dgelqf_worksize
#define INIT_VAR_TREE_OCP_QP d_init_var_tree_ocp_qp
#define MEMSIZE_CORE_QP_IPM d_memsize_core_qp_ipm
#define MEMSIZE_TREE_OCP_QP_RES d_memsize_tree_ocp_qp_res
#define MEMSIZE_TREE_OCP_QP_SOL d_memsize_tree_ocp_qp_sol
#define REAL double
#define SIZE_STRMAT blasfeo_memsize_dmat
#define SIZE_STRVEC blasfeo_memsize_dvec
#define SOLVE_KKT_STEP_TREE_OCP_QP d_solve_kkt_step_tree_ocp_qp
#define STRMAT blasfeo_dmat
#define STRVEC blasfeo_dvec
#define TREE_OCP_QP d_tree_ocp_qp
#define TREE_OCP_QP_DIM d_tree_ocp_qp_dim
#define TREE_OCP_QP_IPM_ARG d_tree_ocp_qp_ipm_arg
#define HPIPM_MODE hpipm_mode
#define TREE_OCP_QP_IPM_WORKSPACE d_tree_ocp_qp_ipm_workspace
#define TREE_OCP_QP_RES d_tree_ocp_qp_res
#define TREE_OCP_QP_RES_WORKSPACE d_tree_ocp_qp_res_workspace
#define TREE_OCP_QP_SOL d_tree_ocp_qp_sol
#define UPDATE_VAR_QP d_update_var_qp
#define VECMULDOT blasfeo_dvecmuldot
#define VECNRM_INF blasfeo_dvecnrm_inf
#define VECSC blasfeo_dvecsc

// arg
#define MEMSIZE_TREE_OCP_QP_IPM_ARG d_memsize_tree_ocp_qp_ipm_arg
#define CREATE_TREE_OCP_QP_IPM_ARG d_create_tree_ocp_qp_ipm_arg
#define SET_DEFAULT_TREE_OCP_QP_IPM_ARG d_set_default_tree_ocp_qp_ipm_arg
#define SET_TREE_OCP_QP_IPM_ARG_ITER_MAX d_set_tree_ocp_qp_ipm_arg_iter_max
#define SET_TREE_OCP_QP_IPM_ARG_MU0 d_set_tree_ocp_qp_ipm_arg_mu0
#define SET_TREE_OCP_QP_IPM_ARG_TOL_STAT d_set_tree_ocp_qp_ipm_arg_tol_stat
#define SET_TREE_OCP_QP_IPM_ARG_TOL_EQ d_set_tree_ocp_qp_ipm_arg_tol_eq
#define SET_TREE_OCP_QP_IPM_ARG_TOL_INEQ d_set_tree_ocp_qp_ipm_arg_tol_ineq
#define SET_TREE_OCP_QP_IPM_ARG_TOL_COMP d_set_tree_ocp_qp_ipm_arg_tol_comp
#define SET_TREE_OCP_QP_IPM_ARG_REG_PRIM d_set_tree_ocp_qp_ipm_arg_reg_prim
// ipm
#define MEMSIZE_TREE_OCP_QP_IPM d_memsize_tree_ocp_qp_ipm
#define CREATE_TREE_OCP_QP_IPM d_create_tree_ocp_qp_ipm
#define GET_TREE_OCP_QP_IPM_ITER d_get_tree_ocp_qp_ipm_iter
#define GET_TREE_OCP_QP_IPM_RES_STAT d_get_tree_ocp_qp_ipm_res_stat
#define GET_TREE_OCP_QP_IPM_RES_EQ d_get_tree_ocp_qp_ipm_res_eq
#define GET_TREE_OCP_QP_IPM_RES_INEQ d_get_tree_ocp_qp_ipm_res_ineq
#define GET_TREE_OCP_QP_IPM_RES_COMP d_get_tree_ocp_qp_ipm_res_comp
#define GET_TREE_OCP_QP_IPM_STAT d_get_tree_ocp_qp_ipm_stat
#define SOLVE_TREE_OCP_QP_IPM d_solve_tree_ocp_qp_ipm
#define SOLVE_TREE_OCP_QP_IPM2 d_solve_tree_ocp_qp_ipm2



#include "x_tree_ocp_qp_ipm.c"
