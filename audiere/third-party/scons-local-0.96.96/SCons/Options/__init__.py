"""engine.SCons.Options

This file defines the Options class that is used to add user-friendly
customizable variables to an SCons build.
"""

#
# Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "/home/scons/scons/branch.0/branch.96/baseline/src/engine/SCons/Options/__init__.py 0.96.96.D001 2007/04/12 14:12:18 knight"

import os.path
import string

import SCons.Errors
import SCons.Util
import SCons.Warnings

from BoolOption import BoolOption  # okay
from EnumOption import EnumOption  # okay
from ListOption import ListOption  # naja
from PackageOption import PackageOption # naja
from PathOption import PathOption # okay


class Options:
    """
    Holds all the options, updates the environment with the variables,
    and renders the help text.
    """
    def __init__(self, files=None, args={}):
        """
        files - [optional] List of option configuration files to load
            (backward compatibility) If a single string is passed it is 
                                     automatically placed in a file list
        """

        self.options = []
        self.args = args
        self.files = None
        if SCons.Util.is_String(files):
           self.files = [ files ]
        elif files:
           self.files = files

    def _do_add(self, key, help="", default=None, validator=None, converter=None):
        class Option:
            pass

        option = Option()
        option.key = key
        option.help = help
        option.default = default
        option.validator = validator
        option.converter = converter

        self.options.append(option)

    def keys(self):
        """
        Returns the keywords for the options
        """
        return map(lambda o: o.key, self.options)

    def Add(self, key, help="", default=None, validator=None, converter=None, **kw):
        """
        Add an option.

        key - the name of the variable, or a list or tuple of arguments
        help - optional help text for the options
        default - optional default value
        validator - optional function that is called to validate the option's value
                    Called with (key, value, environment)
        converter - optional function that is called to convert the option's value before
                    putting it in the environment.
        """

        if SCons.Util.is_List(key) or type(key) == type(()):
            apply(self._do_add, key)
            return

        if not SCons.Util.is_String(key) or \
           not SCons.Util.is_valid_construction_var(key):
            raise SCons.Errors.UserError, "Illegal Options.Add() key `%s'" % str(key)

        self._do_add(key, help, default, validator, converter)

    def AddOptions(self, *optlist):
        """
        Add a list of options.

        Each list element is a tuple/list of arguments to be passed on
        to the underlying method for adding options.
        
        Example:
          opt.AddOptions(
            ('debug', '', 0),
            ('CC', 'The C compiler'),
            ('VALIDATE', 'An option for testing validation', 'notset',
             validator, None),
            )
        """
        for o in optlist:
            apply(self._do_add, o)


    def Update(self, env, args=None):
        """
        Update an environment with the option variables.

        env - the environment to update.
        """

        values = {}

        # first set the defaults:
        for option in self.options:
            if not option.default is None:
                values[option.key] = option.default

        # next set the value specified in the options file
        if self.files:
           for filename in self.files:
              if os.path.exists(filename):
                 execfile(filename, values)

        # finally set the values specified on the command line
        if args is None:
            args = self.args
        values.update(args)

        # put the variables in the environment:
        # (don't copy over variables that are not declared
        #  as options)
        for option in self.options:
            try:
                env[option.key] = values[option.key]
            except KeyError:
                pass

        # Call the convert functions:
        for option in self.options:
            if option.converter and values.has_key(option.key):
                value = env.subst('${%s}'%option.key)
                try:
                    try:
                        env[option.key] = option.converter(value)
                    except TypeError:
                        env[option.key] = option.converter(value, env)
                except ValueError, x:
                    raise SCons.Errors.UserError, 'Error converting option: %s\n%s'%(option.key, x)


        # Finally validate the values:
        for option in self.options:
            if option.validator and values.has_key(option.key):
                option.validator(option.key, env.subst('${%s}'%option.key), env)

    def Save(self, filename, env):
        """
        Saves all the options in the given file.  This file can
        then be used to load the options next run.  This can be used
        to create an option cache file.

        filename - Name of the file to save into
        env - the environment get the option values from
        """

        # Create the file and write out the header
        try:
            fh = open(filename, 'w')

            try:
                # Make an assignment in the file for each option within the environment
                # that was assigned a value other than the default.
                for option in self.options:
                    try:
                        value = env[option.key]
                        try:
                            eval(repr(value))
                        except KeyboardInterrupt:
                            raise
                        except:
                            # Convert stuff that has a repr() that
                            # cannot be evaluated into a string
                            value = SCons.Util.to_String(value)
                        
                        defaultVal = env.subst(SCons.Util.to_String(option.default))
                        if option.converter:
                            defaultVal = option.converter(defaultVal)

                        if str(env.subst('${%s}' % option.key)) != str(defaultVal):
                            fh.write('%s = %s\n' % (option.key, repr(value)))
                    except KeyError:
                        pass
            finally:
                fh.close()

        except IOError, x:
            raise SCons.Errors.UserError, 'Error writing options to file: %s\n%s' % (filename, x)

    def GenerateHelpText(self, env, sort=None):
        """
        Generate the help text for the options.

        env - an environment that is used to get the current values
              of the options.
        """

        if sort:
            options = self.options[:]
            options.sort(lambda x,y,func=sort: func(x.key,y.key))
        else:
            options = self.options

        def format(opt, self=self, env=env):
            if env.has_key(opt.key):
                actual = env.subst('${%s}' % opt.key)
            else:
                actual = None
            return self.FormatOptionHelpText(env, opt.key, opt.help, opt.default, actual)
        lines = filter(None, map(format, options))

        return string.join(lines, '')

    format = '\n%s: %s\n    default: %s\n    actual: %s\n'

    def FormatOptionHelpText(self, env, key, help, default, actual):
        return self.format % (key, help, default, actual)
