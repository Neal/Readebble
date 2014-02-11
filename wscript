top = '.'
out = 'build'

def options(ctx):
	ctx.load('pebble_sdk')

def configure(ctx):
	ctx.load('pebble_sdk')

def build(ctx):
	ctx.load('pebble_sdk')

	ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'), target='pebble-app.elf')

	ctx.pbl_bundle(elf='pebble-app.elf', js=ctx.path.ant_glob('src/js/**/*.js'))
