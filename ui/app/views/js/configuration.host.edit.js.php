<?php declare(strict_types = 0);
/*
** Copyright (C) 2001-2024 Zabbix SIA
**
** This program is free software: you can redistribute it and/or modify it under the terms of
** the GNU Affero General Public License as published by the Free Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
** without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public License along with this program.
** If not, see <https://www.gnu.org/licenses/>.
**/


/**
 * @var CView $this
 */
?>

<script>
	const view = {
		form: null,

		init({form_name, host_interfaces, proxy_groupid, host_is_discovered}) {
			this.form = document.getElementById(form_name);
			this.form.addEventListener('submit', this.events.submit);
			this.form_name = form_name;

			host_edit.init({form_name, host_interfaces, proxy_groupid, host_is_discovered});
			this.initEvents();
		},

		initEvents() {
			this.form.addEventListener('click', (e) => {
				const target = e.target;

				if (target.classList.contains('js-edit-linked-template')) {
					this.editTemplate({templateid: e.target.dataset.templateid});
				}
				else if (target.classList.contains('js-edit-proxy')) {
					this.editProxy(e.target.dataset.proxyid);
				}
				else if (target.classList.contains('js-update-item')) {
					this.editItem(target, target.dataset);
				}
			});
		},

		editItem(target, data) {
			const overlay = PopUp('item.edit', target.dataset, {
				dialogueid: 'item-edit',
				dialogue_class: 'modal-popup-large',
				trigger_element: target,
				prevent_navigation: true
			});

			overlay.$dialogue[0].addEventListener('dialogue.submit', e => {
					const data = e.detail;

					if ('success' in data) {
						postMessageOk(data.success.title);

						if ('messages' in data.success) {
							postMessageDetails('success', data.success.messages);
						}
					}

					location.href = location.href;
				}, {once: true}
			);
		},

		editTemplate(parameters) {
			const overlay = PopUp('template.edit', parameters, {
				dialogueid: 'templates-form',
				dialogue_class: 'modal-popup-large',
				prevent_navigation: true
			});

			overlay.$dialogue[0].addEventListener('dialogue.submit', (e) => {
				postMessageOk(e.detail.title);

				if ('success' in e.detail) {
					postMessageOk(e.detail.success.title);

					if ('messages' in e.detail.success) {
						postMessageDetails('success', e.detail.success.messages);
					}
				}

				location.href = location.href;
			});
		},

		editProxy(proxyid) {
			const overlay = PopUp('popup.proxy.edit', {proxyid}, {
				dialogueid: 'proxy_edit',
				dialogue_class: 'modal-popup-static',
				prevent_navigation: true
			});

			overlay.$dialogue[0].addEventListener('dialogue.submit', (e) => {
				postMessageOk(e.detail.title);

				if ('success' in e.detail) {
					postMessageOk(e.detail.success.title);

					if ('messages' in e.detail.success) {
						postMessageDetails('success', e.detail.success.messages);
					}
				}

				location.href = location.href;
			});
		},

		submit(button) {
			this.setLoading(button);

			const fields = host_edit.preprocessFormFields(getFormFields(this.form), false);
			const curl = new Curl(this.form.getAttribute('action'));

			fetch(curl.getUrl(), {
				method: 'POST',
				headers: {'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8'},
				body: urlEncodeData(fields)
			})
				.then((response) => response.json())
				.then((response) => {
					if ('error' in response) {
						throw {error: response.error};
					}

					postMessageOk(response.success.title);

					if ('messages' in response.success) {
						postMessageDetails('success', response.success.messages);
					}

					const url = new Curl('zabbix.php');
					url.setArgument('action', 'host.list');

					location.href = url.getUrl();
				})
				.catch(this.ajaxExceptionHandler)
				.finally(() => {
					this.unsetLoading();
				});
		},

		clone() {
			const url = new Curl('');
			url.setArgument('clone', 1);

			const fields = host_edit.preprocessFormFields(getFormFields(this.form), true);
			delete fields.sid;

			post(url.getUrl(), fields);
		},

		delete(hostid, button) {
			const confirm_text = button.getAttribute('confirm');
			if (!confirm(confirm_text)) {
				return;
			}

			this.setLoading(button);

			const curl = new Curl('zabbix.php');
			curl.setArgument('action', 'host.massdelete');
			curl.setArgument(CSRF_TOKEN_NAME, <?= json_encode(CCsrfTokenHelper::get('host')) ?>);

			fetch(curl.getUrl(), {
				method: 'POST',
				headers: {'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8'},
				body: urlEncodeData({hostids: [hostid]})
			})
				.then((response) => response.json())
				.then((response) => {
					if ('error' in response) {
						throw {error: response.error};
					}

					postMessageOk(response.success.title);

					if ('messages' in response.success) {
						postMessageDetails('success', response.success.messages);
					}

					const url = new Curl('zabbix.php');
					url.setArgument('action', 'host.list');

					location.href = url.getUrl();
				})
				.catch(this.ajaxExceptionHandler)
				.finally(() => {
					this.unsetLoading();
				});
		},

		setLoading(active_button) {
			active_button.classList.add('is-loading');

			const footer = this.form.querySelector('.tfoot-buttons');

			for (const button of footer.querySelectorAll('button:not(.js-cancel)')) {
				button.disabled = true;
			}
		},

		unsetLoading() {
			const footer = this.form.querySelector('.tfoot-buttons');

			for (const button of footer.querySelectorAll('button:not(.js-cancel)')) {
				button.classList.remove('is-loading');
				button.disabled = false;
			}
		},

		ajaxExceptionHandler: (exception) => {
			clearMessages();

			let title, messages;

			if (typeof exception === 'object' && 'error' in exception) {
				title = exception.error.title;
				messages = exception.error.messages;
			}
			else {
				messages = [<?= json_encode(_('Unexpected server error.')) ?>];
			}

			const message_box = makeMessageBox('bad', messages, title);

			addMessage(message_box);
		},

		events: {
			submit(event) {
				event.preventDefault();
				const submit_button = view.form.querySelector('.tfoot-buttons button[type="submit"]');

				view.submit(submit_button);
			}
		}
	};
</script>
