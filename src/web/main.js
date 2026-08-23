function showPage(pageId, element) {
	// Show content
	const pages = document.querySelectorAll('.page-content');
	pages.forEach(page => page.style.display = 'none');
	document.getElementById(pageId + '-page').style.display = 'block';
	
	// Redraw menu style
	const links = document.querySelectorAll('.menu-link');
	links.forEach(link => link.classList.remove('active'));
	element.classList.add('active');
}

async function fetchUptime()
{
	try {
		const response = await fetch("/uptime");
		if (!response.ok) {
			throw new Error(`Response status: ${response.status}`);
		}

		const json = await response.json();
		const uptime = document.getElementById("uptime");
		uptime.innerHTML = json + " milliseconds"
	} catch (error) {
		console.error(error.message);
	}
}

window.addEventListener("DOMContentLoaded", (ev) => {
	/* Fetch the uptime once per second */
	setInterval(fetchUptime, 1000);

	/* Setup websocket for handling network stats */
	const ws = new WebSocket("/");
})
