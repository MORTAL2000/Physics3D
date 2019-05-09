#include "frameBuffer.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "../../util/log.h"

FrameBuffer::FrameBuffer() {
	glGenFramebuffers(1, &id);
	bind();
}

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height) : FrameBuffer() {
	texture = new Texture(width, height);
	renderBuffer = new RenderBuffer(width, height);
	attach(texture);
	attach(renderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::error("FrameBuffer object with id (%d) not complete", id);

	unbind();
}

FrameBuffer::FrameBuffer(Texture* colorAttachment, RenderBuffer* depthStencilAttachment) : FrameBuffer() {
	attach(colorAttachment);
	attach(depthStencilAttachment);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		Log::error("FrameBuffer object with id (%d) not complete", id);

	unbind();
}

void FrameBuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attach(Texture* texture) {
	bind();
	this->texture = texture;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0);
}

void FrameBuffer::attach(RenderBuffer* renderBuffer) {
	bind();
	this->renderBuffer = renderBuffer;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer->id);
}

void FrameBuffer::close() {
	glDeleteFramebuffers(1, &id);
}


// DepthFrameBuffer

DepthFrameBuffer::DepthFrameBuffer(unsigned int width, unsigned int height) {
	texture = new DepthTexture(width, height);
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::bind() {
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void DepthFrameBuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::close() {
	glDeleteTextures(1, &texture->id);
	glDeleteFramebuffers(1, &id);
}