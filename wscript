#! /usr/bin/env python

import os

# the following two variables are used by the target "waf dist"
VERSION='1.2'
APPNAME='tutte'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(ctx):
	ctx.load('compiler_cxx')
	ctx.load('boost', tooldir = '.')

def configure(ctx):
	ctx.load('compiler_cxx')
	ctx.load('boost')

	ctx.env.append_value('CXXFLAGS', ['-Wall', '-Wno-long-long', '-ansi', '-pedantic', '-O3']) 
	ctx.check_cc(lib = 'gmp')
	ctx.check_cxx(lib = 'gmpxx')

	boost_root = os.environ.get('BOOST_ROOT', None)
	if boost_root:
		ctx.check_boost('program_options',
				includes = boost_root,
				libs = os.path.join(boost_root, 'stage', 'lib'))
	else:
		ctx.check_boost('program_options')
	
	ctx.define('VERSION', VERSION)
	ctx.write_config_header('include/tutteconfig.h') 

def run_test(ctx):
	test_str = './tutte --input-file ${SRC[0].abspath()} 2>/dev/null | diff --ignore-all-space - ${SRC[1].abspath()}'
	for i, o in zip(ctx.path.ant_glob('tests/*.input'), ctx.path.ant_glob('tests/*.output')):
 		ctx(rule = test_str, name = i,
		    source = [i, o],
 		    always = True, shell = True, color = 'PINK')

def build(bld):
	bld.program(features = 'cxx cxxprogram',
		    name     = 'tutte',
		    source   = 'src/tutte.cpp src/parse_graph.cpp',
		    target   = 'tutte',
		    includes = './include',
		    lib      = ['gmp', 'gmpxx'],
		    use      = 'BOOST')
	if bld.cmd == 'test':
		bld.add_group()
		run_test(bld)

from waflib.Build import BuildContext
class test_context(BuildContext):
 	cmd = 'test'
 	fun = 'build'
